// Test (linux) for LWPNG

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>
#include <lwpng.h>

int debug = 0;


const char *
start (void *opaque, uint32_t w, uint32_t h, uint8_t hasalpha)
{
   printf ("W=%u H=%u%s\n", w, h, hasalpha ? " with alpha" : "");
   return NULL;
}

const char *
pixel (void *opaque, uint32_t x, uint32_t y, uint16_t r, uint16_t g, uint16_t b, uint16_t a)
{
#ifndef	DEBUG
   printf ("X=%03u Y=%03u R=%f G=%f B=%f A=%f\n", x, y, (float) r / 65535, (float) g / 65535, (float) b / 65535, (float) a / 65535);
#endif
   return NULL;
}

int
main (int argc, const char *argv[])
{
   if (argc == 2)
   {                            // Decode
      FILE *f = fopen (argv[1], "r");
      if (!f)
         err (1, "Cannot open %s", argv[1]);

      lwpng_decode_t *p = lwpng_decode (NULL, &start, &pixel, NULL, NULL, NULL);
      if (!p)
         errx (1, "Failed to init");

      unsigned char buf[100];
      size_t l;
      while ((l = fread (buf, 1, sizeof (buf), f)) > 0)
         lwpng_data (p, l, buf);
      const char *e = lwpng_decoded (&p);
      fclose (f);
      if (e)
         errx (1, "Failed %s", e);
      return 0;
   }
   // Encode test
   int w = 32,
      h = 32;
   lwpng_encode_t *p = lwpng_encode_1bit (w, h, NULL, NULL, NULL);

   size_t len;
   uint8_t *png;
   uint8_t scan[(w + 7) / 8];
   while (h--)
   {
      for (int p = 0; p < (w + 7) / 8; p++)
         scan[p] = p + h;
      lwpng_encode_scanline (p, scan);
   }

   const char *e = lwpng_encoded (&p, &len, &png);
   if (e)
      errx (1, "Failed %s", e);
   else
      fwrite ((char *) png, 1, len, stdout);

   return 0;
}
