// PNG expander

#include <lwpng.h>
#include <malloc.h>
#include <string.h>
#include <arpa/inet.h>
#ifdef	CONFIG_LWPNG_DEBUG
#ifdef	CONFIG_IDF_TARGET
#define	warnx	ESP_LOGE
#else
#include <err.h>
#endif
#endif

#ifdef	CONFIG_LWPNG_CHECKS
static const uint32_t crc_table[] = {
   0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4,
   0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
   0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9,
   0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
   0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59, 0x26D930AC, 0x51DE003A,
   0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
   0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F,
   0x9FBFE4A5, 0xE8B8D433, 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
   0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
   0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65, 0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
   0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5,
   0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
   0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6,
   0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
   0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1, 0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB,
   0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
   0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA, 0xAF0A1B4C,
   0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
   0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31,
   0x2CD99E8B, 0x5BDEAE1D, 0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
   0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
   0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777, 0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
   0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7,
   0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
   0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8,
   0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};
#endif

static const uint8_t png_signature[] = { 137, 80, 78, 71, 13, 10, 26, 10 };

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


struct lwpng_s
{
   void *opaque;                // Used for callback
   const char *error;           // Set if error state
   mz_stream mz;                // Inflate
   lwpng_cb_start_t *cb_start;  // Call back start
   lwpng_cb_pixel_t *cb_pixel;  // Call back pixel
   uint32_t remaining;          // Bytes remaining in current state
   uint16_t PLTE_len;           // Length of PLTE (bytes)
   uint16_t tRNS_len;           // Length of tRNS (bytes)
#ifdef	CONFIG_LWPNG_CHECKS
   uint32_t crc;
   uint32_t crccheck;
#endif
   struct __attribute__((__packed__))
   {
      uint32_t length;
      uint8_t type[4];
   } chunk;
   struct __attribute__((__packed__))
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
   uint8_t state:4;             // Current state
#ifdef	CONFIG_LWPNG_CHECKS
   uint8_t start:1;             // We have started (had IHDR)
   uint8_t data:1;              // We have had IDAT
#endif
   uint8_t end:1;               // We have cleanly got to the end - retain even if CHECKS not set as file could be cut short
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

#if 0
static const char *
scan_byte (lwpng_t * p, uint8_t b)
{                               // Process inflated IDAT byte
   // TODO
   return NULL;
}
#endif

static const char *
idat_byte (lwpng_t * p, uint8_t b)
{                               // process byte from IDAT, compressed
   // TODO inflate
   return NULL;
}

static const char *
png_byte (lwpng_t * p, uint8_t b)
{                               // process byte from PNG file
#ifdef	CONFIG_LWPNG_CHECKS
   if (p->state != STATE_CRC && p->end)
   {
#ifdef	CONFIG_LWPNG_DEBUG
      warnx ("Data after end %02X", b);
#endif
      return "Data after end";
   }
#endif
   p->remaining--;
#ifdef	CONFIG_LWPNG_CHECKS
   if (p->state != STATE_CRC)
      p->crc = crc_table[(p->crc ^ b) & 0xff] ^ (p->crc >> 8);
#endif
   switch (p->state)
   {
   case STATE_SIGNATURE:
      if (b != png_signature[sizeof (png_signature) - p->remaining - 1])
         return "Bad signature";        // This stays even when CHECKS not set - because we may have been sent a non PNG file
      break;
   case STATE_CRC:             // CRC already handled
#ifdef	CONFIG_LWPNG_CHECKS
      p->crccheck = (p->crccheck << 8) | b;
#endif
      break;
   case STATE_CHUNK:
      ((uint8_t *) & p->chunk)[sizeof (p->chunk) - p->remaining - 1] = b;
#ifdef	CONFIG_LWPNG_CHECKS
      if (p->remaining == 4)
         p->crc = 0xffffffff;   // Start CRC
#endif
      break;
   case STATE_IHDR:
      ((uint8_t *) & p->IHDR)[sizeof (p->IHDR) - p->remaining - 1] = b;
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
      if (p->state == STATE_SIGNATURE)
      {
         p->state = STATE_CHUNK;        // ready for first chunk
         p->remaining = sizeof (p->chunk);
      }
      else
      if (p->state == STATE_CRC)
      {                         // CRC done
#ifdef	CONFIG_LWPNG_CHECKS
         if (p->crc != (p->crccheck ^ 0xFFFFFFFF))
         {
#ifdef	CONFIG_LWPNG_DEBUG
            warnx ("Chunk %.4s CRC %08X %08X", p->chunk.type, p->crc, p->crccheck ^ 0xFFFFFFFF);
#endif
            return "Bad CRC";
         }
#endif
         p->state = STATE_CHUNK;        // ready for next chunk
         p->remaining = sizeof (p->chunk);
      }
      else if (p->state == STATE_CHUNK)
      {                         // Start of chunk - work out type
         p->remaining = p->chunk.length = ntohl (p->chunk.length);
#ifdef	CONFIG_LWPNG_DEBUG
         warnx ("Chunk %.4s Len %u", p->chunk.type, p->chunk.length);
#endif
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
#ifdef	CONFIG_LWPNG_CHECKS
               p->data = 1;
#endif
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
            }
#ifdef	CONFIG_LWPNG_CHECKS
            else if (!(p->chunk.type[0] & 0x20))
               return "Unknown critical chunk";
#endif
            else
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
            if (p->IHDR.colour != 0 && p->IHDR.colour != 2 && p->IHDR.colour != 3 && p->IHDR.colour != 4 && p->IHDR.colour != 6)
               return "Invalid colour";
            if (p->IHDR.depth != 1 && p->IHDR.depth != 2 && p->IHDR.depth != 4 && p->IHDR.depth != 8 && p->IHDR.depth != 16)
               return "Invalid depth";
            if ((p->IHDR.colour && p->IHDR.colour != 3 && p->IHDR.depth < 8) || (p->IHDR.colour == 3 && p->IHDR.depth > 8))
               return "Invalid colour+depth";
            if (p->IHDR.compress)
               return "Invalid compression";
            if (p->IHDR.filter)
               return "Invalid filter";
            if (p->IHDR.interlace > 1)
               return "Invalid interlace";
#endif
            if (p->cb_start)
               p->cb_start (p->opaque, p->IHDR.width, p->IHDR.height, p->IHDR.colour & 4);
         }
      }
      if (!p->remaining)
      {
         p->state = STATE_CRC;
         p->remaining = 4;
      }
   }
   return p->error;
}

// Calls to decode PNG
// Note, once an error happens it latches and all further lwpng_data calls are ignored, as such you can just check error on lwpng_end

// Allocate a new PNG decode, alloc/free can be NULL for system defaults
lwpng_t *
lwpng_init (void *opaque, lwpng_cb_start_t * start, lwpng_cb_pixel_t * pixel, mz_alloc_func zalloc, mz_free_func zfree,
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
   p->cb_start = start;
   p->cb_pixel = pixel;
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
