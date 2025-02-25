// PNG expander

#include <sdkconfig.h>
#include <lwpng.h>
#include <malloc.h>
#include <string.h>
#include <arpa/inet.h>

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

static const uint8_t adam7x[] = { 0, 0, 4, 0, 2, 0, 1, 0 };     // X start
static const uint8_t adam7xstep[] = { 1, 8, 8, 4, 4, 2, 2, 1 }; // X step
static const uint8_t adam7y[] = { 0, 0, 0, 4, 0, 2, 0, 1 };     // Y start
static const uint8_t adam7ystep[] = { 1, 8, 8, 8, 4, 4, 2, 2 }; // Y step

#define	COLOUR_PALETTE	1
#define	COLOUR_RGB	2
#define	COLOUR_ALPHA	4

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
   z_stream z;                  // Inflate
   lwpng_cb_start_t *cb_start;  // Call back start
   lwpng_cb_pixel_t *cb_pixel;  // Call back pixel
   uint32_t remaining;          // Bytes remaining in current state
   uint16_t PLTE_len;           // Length of PLTE (bytes)
   uint16_t tRNS_len;           // Length of tRNS (bytes)
#ifdef	CONFIG_LWPNG_CHECKS
   uint32_t crc;
   uint32_t crccheck;
#endif
   uint8_t *PLTE;               // Malloc'd PLTE
   uint8_t *tRNS;               // Malloc'd tRNS
   uint8_t *scan;               // Malloc'd scan line (max bytes)
   uint8_t pixel[8];            // Last pixel for filter (worst case is 16 bit R G B A)
   uint32_t y;                  // Y position from top
   uint32_t x;                  // X position from left
   uint32_t ppos;               // filter pixel pos
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
   uint8_t state:4;             // Current state
   uint8_t bpp:4;               // BPP for filter
   uint8_t bpos:4;              // Byte in bpp for filter
   uint8_t adam7:3;             // Which interlace
   uint8_t filter:3;            // This line filter
#ifdef	CONFIG_LWPNG_CHECKS
   uint8_t start:1;             // We have started (had IHDR)
#endif
   uint8_t data:1;              // We have had IDAT
   uint8_t end:1;               // We have cleanly got to the end - retain even if CHECKS not set as file could be cut short
};

static void *
lwpng_alloc (void *opaque, uInt items, uInt size)
{
   return malloc (items * size);
}

static void
lwpng_free (void *opaque, void *address)
{
   free (address);
}

static inline uint8_t
paeth (uint8_t a, uint8_t b, uint8_t c)
{
   uint16_t p = a + b + c;
   uint16_t pa = (p > a ? p - a : a - p);
   uint16_t pb = (p > b ? p - b : b - p);
   uint16_t pc = (p > c ? p - c : c - p);
   if (pa <= pb && pa <= pc)
      return a;
   if (pb <= pc)
      return b;
   return c;
}

static void
pixels (lwpng_t * p)
{
   if (p->bpp == 1 && p->IHDR.depth < 8 && ((p->IHDR.colour ^ COLOUR_RGB) & (COLOUR_PALETTE | COLOUR_RGB)))
   {                            // Multiple pixel per byte
      uint8_t B = p->pixel[0];
      uint8_t step = adam7xstep[p->adam7];
      uint8_t steps = 8 / p->IHDR.depth;
      uint32_t x = p->x;
      while (steps-- && !p->error && p->x < p->IHDR.width)
      {
         uint16_t r = 0,
            g = 0,
            b = 0,
            a = 0xFFFF;
         uint8_t v = (B >> (8 - p->IHDR.depth));
         if (p->IHDR.colour & COLOUR_PALETTE)
         {                      // Palette
            if (!p->PLTE || v > p->PLTE_len / 3)
            {
               a = 0;           // Too big, this is an error
#ifdef	CONFIG_LWPNG_CHECKS
               p->error = "Invalid palette entry";
#endif
            } else
            {
               r = p->PLTE[v * 3 + 0] * 257;
               g = p->PLTE[v * 3 + 1] * 257;
               b = p->PLTE[v * 3 + 2] * 257;
               if (p->tRNS && v < p->tRNS_len)
                  a = p->tRNS[v] * 257;
            }
         } else
         {                      // Grey
            if (p->IHDR.depth == 1)
               r = g = b = v * 0xFFFF;
            else if (p->IHDR.depth == 2)
               r = g = b = v * 0xAAAA;
            else if (p->IHDR.depth == 4)
               r = g = b = v * 0x8888;
            else
               r = g = b = v * 0x8080;
            if (p->tRNS && p->tRNS_len == 2 && r == p->tRNS[0] * 256 + p->tRNS[1])
               a = 0;
         }
         if (p->cb_pixel)
            p->error = p->cb_pixel (p->opaque, x, p->y, r, g, b, a);
         B <<= p->IHDR.depth;
         x += step;
      }
   } else
   {                            // Single pixel
      uint8_t *x = p->pixel;
      uint16_t r,
        g,
        b,
        a = 0xFFFF;
      if (p->IHDR.depth == 16)
      {                         // 16 bit
         if (p->IHDR.colour & COLOUR_RGB)
         {
            r = *x++;
            r = (r << 8) + *x++;
            g = *x++;
            g = (g << 8) + *x++;
            b = *x++;
            r = (b << 8) + *x++;
         } else
         {
            r = *x++;
            r = (r << 8) + *x++;
            g = b = r;
         }
         if (p->IHDR.colour & COLOUR_ALPHA)
         {
            a = *x++;
            a = (a << 8) + *x++;
         }
      } else
      {                         // 8 bit
         if (p->IHDR.colour & COLOUR_RGB)
         {
            r = 257 * *x++;
            g = 257 * *x++;
            b = 257 * *x++;
         } else
            r = g = b = 257 * *x++;
         if (p->IHDR.colour & COLOUR_ALPHA)
            a = 257 * *x++;
      }
      if (p->tRNS && p->tRNS_len == 6 && r == p->tRNS[0] * 256 + p->tRNS[1] && g == p->tRNS[2] * 256 + p->tRNS[3]
          && b == p->tRNS[4] * 256 + p->tRNS[4])
         a = 0;
      if (p->cb_pixel)
         p->error = p->cb_pixel (p->opaque, p->x, p->y, r, g, b, a);
   }
}

static const char *
scan_byte (lwpng_t * p, uint8_t b)
{                               // Process inflated IDAT byte
   if (p->filter == 7)
   {                            // Start of scan line
      if (b > 4)
         return "Bad filter";
      p->filter = b;
      p->ppos = 0;
#ifdef	DEBUG
      if (p->adam7)
         printf ("A%u", p->adam7);
      printf ("Y%u", p->y);
      if (p->x)
         printf ("X%u", p->x);
      if (p->filter)
         printf ("F%u", p->filter);
      printf (":");
#endif
      return NULL;
   }
   uint8_t l = (p->ppos ? p->pixel[p->bpos] : 0);
   uint8_t ul = (p->ppos > 0 ? p->scan[(uint64_t) (p->ppos - 1) * p->bpp + p->bpos] : 0);
   uint8_t u = p->scan[(uint64_t) p->ppos * p->bpp + p->bpos];
   switch (p->filter)
   {
   case 0:                     // None
      break;
   case 1:                     // Sub
      b += l;
      break;
   case 2:                     // Up
      b += u;
      break;
   case 3:                     // Average
      b += ((uint16_t) u + l) / 2;
      break;
   case 4:                     // Paeth
      b += paeth (l, u, ul);
      break;
   }
#ifdef	DEBUG
   printf ("%02X", b);
#endif
   if (p->ppos > 0)
      p->scan[(uint64_t) (p->ppos - 1) * p->bpp + p->bpos] = p->pixel[p->bpos];
   p->pixel[p->bpos] = b;
   p->bpos++;
   if (p->bpos == p->bpp)
   {                            // End of pixel
      pixels (p);
#ifdef	DEBUG
      printf (" ");
#endif
      p->bpos = 0;
      uint8_t step = adam7xstep[p->adam7];
      if (p->bpp == 1 && p->IHDR.depth < 8 && ((p->IHDR.colour ^ COLOUR_RGB) & (COLOUR_PALETTE | COLOUR_RGB)))
         p->x += step * 8 / p->IHDR.depth;      // multiple bytes
      else
         p->x += step;
      p->ppos++;
      if (p->x >= p->IHDR.width)
      {                         // End of line
         memcpy (p->scan + (uint64_t) (p->ppos - 1) * p->bpp, p->pixel, p->bpp);        // Last pixel stored
         // Next line
         p->y += adam7ystep[p->adam7];
         p->x = adam7x[p->adam7];
         if (p->adam7 && (p->x >= p->IHDR.width || p->y >= p->IHDR.height))
         {                      // End of adam7 scan
            uint64_t bytes = 0;
            if (((p->IHDR.colour ^ COLOUR_RGB) & (COLOUR_PALETTE | COLOUR_RGB)))
               bytes = ((uint64_t) p->IHDR.width * 8 + 7) / p->IHDR.depth;
            else
               bytes = (uint64_t) p->IHDR.width * p->bpp;
            memset (p->scan, 0, bytes); // Clean for new scan
            while (p->adam7 && (p->x >= p->IHDR.width || p->y >= p->IHDR.height))
            {                   // Skip non present scan lines
               p->adam7++;
               p->x = adam7x[p->adam7];
               p->y = adam7y[p->adam7];
            }
         }
         p->filter = 7;         // Flag start of line
#ifdef	DEBUG
         printf ("\n");
#endif
      }
   }
   return NULL;
}

static const char *
idat_bytes (lwpng_t * p, uint32_t len, uint8_t * in)
{                               // process bytes from IDAT, compressed
   p->z.next_in = in;
   p->z.avail_in = len;
   p->z.total_in = 0;
#ifdef	DEBUG
   uint32_t o = 0;
#endif
   do
   {
      uint8_t out[16];
      p->z.next_out = out;
      p->z.avail_out = sizeof (out);
      p->z.total_out = 0;
      int e = inflate (&p->z, 0);
      if (e != Z_OK && e != Z_STREAM_END && e != Z_BUF_ERROR)
         return "Inflate not OK";
      for (int i = 0; i < p->z.total_out; i++)
         scan_byte (p, out[i]);
#ifdef	DEBUG
      o += p->z.total_out;
#endif
   }
   while (p->z.avail_in || !p->z.avail_out);
#ifdef	DEBUG
   printf ("Inflate In=%u out=%u\n", len, o);
#endif
   return NULL;
}

static const char *
png_bytes (lwpng_t * p, uint32_t len, uint8_t * in)
{                               // process byte from PNG file - up to p->remaining only
#ifdef	CONFIG_LWPNG_CHECKS
   if (p->state != STATE_CRC && p->end)
      return "Data after end";
#endif
#ifdef	CONFIG_LWPNG_CHECKS
   if (p->state != STATE_CRC)
      for (uint32_t i = 0; i < len; i++)
         p->crc = crc_table[(p->crc ^ in[i]) & 0xff] ^ (p->crc >> 8);
#endif
   switch (p->state)
   {
   case STATE_SIGNATURE:
      if (memcmp (png_signature + sizeof (png_signature) - p->remaining, in, len))
         return "Bad signature";        // This stays even when CHECKS not set - because we may have been sent a non PNG file
      break;
   case STATE_CRC:             // CRC already handled
#ifdef	CONFIG_LWPNG_CHECKS
      for (uint32_t i = 0; i < len; i++)
         p->crccheck = (p->crccheck << 8) | in[i];
#endif
      break;
   case STATE_CHUNK:
      memcpy (((uint8_t *) & p->chunk) + sizeof (p->chunk) - p->remaining, in, len);
      break;
   case STATE_IHDR:
      memcpy (((uint8_t *) & p->IHDR) + sizeof (p->IHDR) - p->remaining, in, len);
      break;
   case STATE_PLTE:
      memcpy (p->PLTE + p->PLTE_len, in, len);
      p->PLTE_len += len;
      break;
   case STATE_tRNS:
      memcpy (p->tRNS + p->tRNS_len, in, len);
      p->tRNS_len += len;
      break;
   case STATE_IDAT:
      p->error = idat_bytes (p, len, in);
      break;
   case STATE_DISCARD:
      break;
#ifdef	CONFIG_LWPNG_CHECKS
   default:
      return "Bad state";
#endif
   }
   p->remaining -= len;
   if (!p->remaining)
   {                            // next state
      if (p->state == STATE_SIGNATURE)
      {
         p->state = STATE_CHUNK;        // ready for first chunk
         p->remaining = sizeof (p->chunk);
      } else if (p->state == STATE_CRC)
      {                         // CRC done
#ifdef	CONFIG_LWPNG_CHECKS
         if (p->crc != (p->crccheck ^ 0xFFFFFFFF))
            return "Bad CRC";
#endif
         p->state = STATE_CHUNK;        // ready for next chunk
         p->remaining = sizeof (p->chunk);
      } else if (p->state == STATE_CHUNK)
      {                         // Start of chunk - work out type
#ifdef	CONFIG_LWPNG_CHECKS
         p->crc = 0xFFFFFFFF;   // Start CRC
         for (uint32_t i = 0; i < 4; i++)
            p->crc = crc_table[(p->crc ^ p->chunk.type[i]) & 0xff] ^ (p->crc >> 8);
#endif
         p->remaining = p->chunk.length = ntohl (p->chunk.length);
#ifdef	DEBUG
         printf ("Chunk %.4s Len %u\n", p->chunk.type, p->chunk.length);
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
               if (!p->data)
               {                // Start
                  p->data = 1;
                  uint64_t bytes = 0;
                  if (((p->IHDR.colour ^ COLOUR_RGB) & (COLOUR_PALETTE | COLOUR_RGB)))
                     p->bpp = ((p->IHDR.depth / 8) ? : 1);      // Indexed or grey, may be packed, grey may have alpha
                  else
                     p->bpp = (p->IHDR.depth / 8) * 3;  // Colour per pixel (8 or 16 bits) possibly with alpha
                  if ((p->IHDR.colour & (COLOUR_PALETTE | COLOUR_ALPHA)) == COLOUR_ALPHA)
                     p->bpp += (p->IHDR.depth / 8);     // Alpha non indexed
                  if (p->IHDR.depth < 8)
                     bytes = ((uint64_t) p->IHDR.width * 8 + 7) / p->IHDR.depth;
                  else
                     bytes = (uint64_t) p->IHDR.width * p->bpp;
#ifdef	DEBUG
                  printf ("Bytes %lu BPP %u\n", bytes, p->bpp);
#endif
                  if (!(p->scan = p->z.zalloc (p->z.opaque, 1, bytes)))
                     return "Out of memory";
                  memset (p->scan, 0, bytes);
                  p->filter = 7;        // Start of scan line
                  if (p->IHDR.interlace)
                     p->adam7 = 1;
               }
               p->state = STATE_IDAT;
            } else if (!memcmp (p->chunk.type, "PLTE", 4))
            {
#ifdef	CONFIG_LWPNG_CHECKS
               if (p->data)
                  return "PLTE too late";
               if (p->PLTE)
                  return "Duplicate PLTE";
               if (p->remaining < 3 || p->remaining > 256 * 3 || p->remaining % 3)
                  return "Bad PLTE length";
               if (!(p->IHDR.colour & COLOUR_RGB))
                  return "PLTE not expected";
#endif
               if (p->IHDR.colour & COLOUR_PALETTE)
               {
                  p->state = STATE_PLTE;
                  if (!(p->PLTE = p->z.zalloc (p->z.opaque, 1, p->remaining)))
                     return "Out of memory";
               } else
                  p->state = STATE_DISCARD;     // Pallette not used
            } else if (!memcmp (p->chunk.type, "tRNS", 4))
            {
#ifdef	CONFIG_LWPNG_CHECKS
               if (p->data)
                  return "tRNS too late";
               if (p->tRNS)
                  return "Duplicate tRNS";
               if (!p->remaining || p->remaining > 256 || (!p->IHDR.colour && p->remaining != 2)
                   || (p->IHDR.colour == COLOUR_RGB && p->remaining != 6))
                  return "Bad tRNS length";
               if (p->IHDR.colour & COLOUR_ALPHA)
                  return "tRNS not expected";
#endif
               p->state = STATE_tRNS;
               if (!(p->tRNS = p->z.zalloc (p->z.opaque, 1, p->remaining)))
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
            if (p->IHDR.colour != 0 && p->IHDR.colour != COLOUR_RGB && p->IHDR.colour != (COLOUR_PALETTE | COLOUR_RGB)
                && p->IHDR.colour != COLOUR_ALPHA && p->IHDR.colour != (COLOUR_ALPHA | COLOUR_RGB))
               return "Invalid colour";
            if (p->IHDR.depth != 1 && p->IHDR.depth != 2 && p->IHDR.depth != 4 && p->IHDR.depth != 8 && p->IHDR.depth != 16)
               return "Invalid depth";
            if ((p->IHDR.colour && p->IHDR.colour != (COLOUR_PALETTE | COLOUR_RGB) && p->IHDR.depth < 8)
                || (p->IHDR.colour == (COLOUR_PALETTE | COLOUR_RGB) && p->IHDR.depth > 8))
               return "Invalid colour+depth";
            if (p->IHDR.compress)
               return "Invalid compression";
            if (p->IHDR.filter)
               return "Invalid filter";
            if (p->IHDR.interlace > 1)
               return "Invalid interlace";
#endif
            if (p->cb_start)
            {
#ifdef	DEBUG
               printf ("W=%d H=%d colour=%02X depth=%02X compress=%02X filter=%02X interlace=%02X\n", p->IHDR.width, p->IHDR.height,
                       p->IHDR.colour, p->IHDR.depth, p->IHDR.compress, p->IHDR.filter, p->IHDR.interlace);
#endif
               p->error = p->cb_start (p->opaque, p->IHDR.width, p->IHDR.height, p->IHDR.colour & COLOUR_ALPHA);
            }
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
lwpng_init (void *opaque, lwpng_cb_start_t * start, lwpng_cb_pixel_t * pixel, alloc_func zalloc, free_func zfree, void *allocopaque)
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
   p->z.zalloc = zalloc;
   p->z.zfree = zfree;
   p->z.opaque = allocopaque;
   p->remaining = sizeof (png_signature);
   if (inflateInit (&p->z) != Z_OK)
      p->error = "Inflate init error";
   return p;
}

// Process data sequentially as data received for PNG file, returns NULL if OK, else error string
const char *
lwpng_data (lwpng_t * p, size_t len, uint8_t * data)
{
   if (!p)
      return "No control structure";
   while (!p->error && len)
   {
      uint32_t q = p->remaining;        // Up to remaining bytes at a time
      if (q > len)
         q = len;
      p->error = png_bytes (p, q, data);
      len -= q;
      data += q;
   }
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
      p->z.zfree (p->opaque, p->PLTE);
   if (p->tRNS)
      p->z.zfree (p->opaque, p->tRNS);
   if (p->scan)
      p->z.zfree (p->opaque, p->scan);
   inflateEnd (&p->z);
   p->z.zfree (p->z.opaque, p);
   return e;
}

const char *
lwpng_get_info (uint32_t len, uint8_t * data, uint32_t * w, uint32_t * h)
{                               // Get file header data
   if (!data || len < 29)
      return "Not enough data";
   if (memcmp (data, png_signature, sizeof (png_signature)))
      return "Not a PNG";
   if (memcmp (data + sizeof (png_signature) + 4, "IHDR", 4))
      return "Missing IHDR";
   if (ntohl (*(uint32_t *) (data + sizeof (png_signature))) != 13)
      return "Bad IHDR len";
   if (w)
      *w = ntohl (*(uint32_t *) (data + sizeof (png_signature) + 8));
   if (h)
      *h = ntohl (*(uint32_t *) (data + sizeof (png_signature) + 8 + 4));
   return NULL;
}
