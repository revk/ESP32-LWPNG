// PNG expander

#include <lwpng.h>
#include <malloc.h>
#include <string.h>
#include <arpa/inet.h>

enum
{
   STATE_SIGNATURE,             // Loading signature
   STATE_CHUNK,                 // Loading the chunk header
   STATE_CRC,                   // Loading the final CRC
   STATE_IHDR,                  // Loading IHDR
   STATE_PLTE,                  // Loading PLTE
   STATE_tRNS,                  // Loading tRNS
   STATE_IDAT,                  // Loading IDAT
   STATE_DISCARD,               // Discarding chunk
};

static const uint8_t png_signature[] = { 137, 80, 78, 71, 13, 10, 26, 10 };

struct lwpng_s
{
   void *opaque;                // Used for callback
   const char *error;           // Set if error state
   mz_stream mz;                // Inflate
   uint32_t remaining;          // Bytes remaining in current state
   uint16_t PLTE_len;           // Length of PLTE (bytes)
   uint16_t tRNS_len;           // Length of tRNS (bytes)
   uint8_t state:4;             // Current state
#ifdef	CONFIG_LWPNG_CHECKS
   uint8_t start:1;             // We have started (had IHDR)
#endif
   uint8_t data:1;              // We have had IDAT
   uint8_t end:1;               // We have cleanly got to the end - retain even if CHECKS not set as file could be cut short
#ifdef	CONFIG_LWPNG_CHECKS
   uint32_t crc;
#endif
   struct
   {
      uint32_t length;
      uint8_t type[4];
   } chunk;
   struct
   {
      uint32_t width;
      uint32_t height;
      uint8_t depth;
      uint8_t colour;
      uint8_t compress;
      uint8_t filter;
      uint8_t interlace;
   } IHDR;
   uint8_t *PLTE;               // Malloc'd PLTE
   uint8_t *tRNS;               // Malloc'd tRNS
};

static void *
lwpng_alloc (void *opaque, mz_alloc_int items, mz_alloc_int size)
{
   return malloc (items * size);
}

static void
lwpng_free (void *opaque, void *address)
{
   free (address);
}

static const char *
idat_byte (lwpng_t * p, uint8_t b)
{
   // TODO
   return NULL;
}

static const char *
png_byte (lwpng_t * p, uint8_t b)
{                               // process byte
#ifdef	CONFIG_LWPNG_CHECKS
   if (p->end)
      return "Data after end";
#endif
   p->remaining--;
#ifdef	CONFIG_LWPNG_CHECKS
   // TODO crc
#endif
   switch (p->state)
   {
   case STATE_SIGNATURE:
      if (b != png_signature[sizeof (png_signature) - p->remaining])
         return "Bad signature";        // This stays even when CHECKS not set - because we may have been sent a non PNG file
      break;
   case STATE_CRC:             // CRC already handled
      break;
   case STATE_IHDR:
      ((uint8_t *) & p->IHDR)[sizeof (p->IHDR) - p->remaining] = b;
      break;
   case STATE_PLTE:
      p->PLTE[p->PLTE_len++] = b;
      break;
   case STATE_tRNS:
      p->tRNS[p->tRNS_len++] = b;
      break;
   case STATE_IDAT:
      p->error = idat_byte (p, b);
      break;
   case STATE_DISCARD:
      break;
#ifdef	CONFIG_LWPNG_CHECKS
   default:
      return "Bad state";
#endif
   }
   if (!p->remaining)
   {                            // next state
#ifdef	CONFIG_LWPNG_CHECKS
      if (p->state == STATE_CRC && p->crc)
         return "Bad CRC";
#endif
      if (p->state == STATE_CHUNK)
      {                         // Start of chunk
         p->remaining = p->chunk.length = ntohl (p->chunk.length);
         if (!memcmp (p->chunk.type, "IHDR", 4))
         {
#ifdef	CONFIG_LWPNG_CHECKS
            if (p->start)
               return "Duplicate IHDR";
            if (p->remaining != sizeof (p->IHDR))
               return "Bad IHDR len";
            p->start = 1;
#endif
            p->state = STATE_IHDR;
         } else
         {
#ifdef	CONFIG_LWPNG_CHECKS
            if (!p->start)
               return "Missing IHDR";
#endif
            if (!memcmp (p->chunk.type, "IDAT", 4))
            {
               if (!p->data)
               {                // Start
                  p->data = 1;
                  // TODO start callback
               }
               p->state = STATE_IDAT;
            } else if (!memcmp (p->chunk.type, "PLTE", 4))
            {
#ifdef	CONFIG_LWPNG_CHECKS
               if (p->data)
                  return "PLTE too late";
               if (p->PLTE)
                  return "Duplicate PLTE";
#endif
               p->state = STATE_PLTE;
               if (!(p->PLTE = p->mz.zalloc (p->mz.opaque, 1, p->remaining)))
                  return "Out of memory";
            } else if (!memcmp (p->chunk.type, "tRNS", 4))
            {
#ifdef	CONFIG_LWPNG_CHECKS
               if (p->data)
                  return "tRNS too late";
               if (p->tRNS)
                  return "Duplicate tRNS";
#endif
               p->state = STATE_tRNS;
               if (!(p->tRNS = p->mz.zalloc (p->mz.opaque, 1, p->remaining)))
                  return "Out of memory";
            } else if (!memcmp (p->chunk.type, "IEND", 4))
            {
#ifdef	CONFIG_LWPNG_CHECKS
               if (p->remaining)
                  return "Bad IEND";
#endif
               p->end = 1;
            } else
               p->state = STATE_DISCARD;
         }
      } else if (p->state > STATE_CRC)
      {                         // End of chunk
         if (p->state == STATE_IHDR)
         {
            p->IHDR.width = ntohl (p->IHDR.width);
            p->IHDR.height = ntohl (p->IHDR.height);
#ifdef	CONFIG_LWPNG_CHECKS
            if (!p->IHDR.width || (p->IHDR.width & 0x80000000))
               return "Invalid width";
            if (!p->IHDR.height || (p->IHDR.height & 0x80000000))
               return "Invalid height";
            if (p->IHDR.depth != 1 && p->IHDR.depth != 2 && p->IHDR.depth != 4 && p->IHDR.depth != 8 && p->IHDR.depth != 16)
               return "Invalid depth";
            if (p->IHDR.colour != 0 && p->IHDR.colour != 2 && p->IHDR.colour != 3 && p->IHDR.colour != 4 && p->IHDR.colour != 6)
               return "Invalid colour";
            if (p->IHDR.compress)
               return "Invalid compression";
            if (p->IHDR.filter)
               return "Invalid filter";
            if (p->IHDR.interlace > 1)
               return "Invalid interlace";
#endif
         }
         p->state = STATE_CRC;
#ifdef	CONFIG_LWPNG_CHECKS
         p->remaining = sizeof (p->crc);;
#endif
      }
      if (!p->remaining)
      {                         // Next chunk
         p->state = STATE_CHUNK;
         p->remaining = sizeof (p->chunk);
      }
   }
   return p->error;
}

// Calls to decode PNG
// Note, once an error happens it latches and all further lwpng_data calls are ignored, as such you can just check error on lwpng_end

// Allocate a new PNG decode, alloc/free can be NULL for system defaults
lwpng_t *
lwpng_init (void *opaque, lwpng_cb_start_t * start, lwpng_cb_pixel_t * pizel, mz_alloc_func zalloc, mz_free_func zfree,
            void *allocopaque)
{
   if (!zalloc)
      zalloc = lwpng_alloc;
   if (!zfree)
      zfree = lwpng_free;
   lwpng_t *p = zalloc (allocopaque, 1, sizeof (*p));
   if (!p)
      return p;
   memset (p, 0, sizeof (*p));
   p->opaque = opaque;
   p->mz.zalloc = zalloc;
   p->mz.zfree = zfree;
   p->mz.opaque = allocopaque;
   p->remaining = sizeof (png_signature);
   return p;
}

// Process data sequentially as data received for PNG file, returns NULL if OK, else error string
const char *
lwpng_data (lwpng_t * p, size_t len, uint8_t * data)
{
   if (!p)
      return "No control structure";
   while (!p->error && len--)
      p->error = png_byte (p, *data++);
   return p->error;
}

// End processing, frees the control structure, returns NULL if OK, else error string
const char *
lwpng_end (lwpng_t ** pp)
{
   if (!pp || !*pp)
      return "No control structure";
   lwpng_t *p = *pp;
   *pp = NULL;
   const char *e = p->error;
   if (!e && !p->end)
      e = "Unclean end";
   if (p->PLTE)
      p->mz.zfree (p->opaque, p->PLTE);
   if (p->tRNS)
      p->mz.zfree (p->opaque, p->tRNS);
   p->mz.zfree (p->mz.opaque, p);
   return e;
}
