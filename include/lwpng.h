// PNG expander

#include <stdint.h>
#include <zlib.h>

#ifdef	CONFIG_LWPNG_DECODE

typedef	struct	lwpng_decode_s	lwpng_decode_t; // This is the internal (malloc'd) structure for png decoding or encoding

// Callbacks - return NULL for OK, else return an error string

// Called at start of first IDAT
typedef const char *lwpng_cb_start_t(void *opaque,uint32_t w,uint32_t h,uint8_t hasalpha);

// Called for each pixel, values all scaled to uint16_t, no gamma/etc adjustment
// Note the order may not be sequential if interlaced, so should not be assumed
typedef	const char *lwpng_cb_pixel_t(void *opaque,uint32_t x,uint32_t y,uint16_t r,uint16_t g,uint16_t b,uint16_t a);

// Calls to decode PNG
// Note, once an error happens it latches and all further lwpng_data calls are ignored, as such you can just check error on lwpng_end

// Allocate a new PNG decode, alloc/free can be NULL for system defaults
lwpng_decode_t *lwpng_decode(void *opaque,lwpng_cb_start_t*,lwpng_cb_pixel_t*,alloc_func,free_func,void *allocopaque);

// Process data sequentially as data received for PNG file, returns NULL if OK, else error string
const char *lwpng_data(lwpng_decode_t*,size_t len,uint8_t *data);

// End processing, frees the control structure, returns NULL if OK, else error string
const char *lwpng_decoded(lwpng_decode_t**);

// Get info from memory buffer with png in it, at least 29 bytes of data needed
const char*lwpng_get_info(uint32_t len,uint8_t *data,uint32_t *w,uint32_t *h);

#endif

#ifdef	CONFIGF_LWPNG_ENCODE

// Encode is to memory...

typedef	struct	lwpng_encode_s	lwpng_encode_t; // This is the internal (malloc'd) structure for png decoding or encoding

// Allocate a new PNG encode, alloc/free can be NULL for system defaults
lwpng_encode_t *lwpng_encode_1bit(uint32_t w,uint32_t h,alloc_func,free_func,void *allocopaque);

// Write scan line - raw data as per PNG
const char *lwpng_encode_scanline(lwpng_t*,uint8_t *data);

// Get final PNG, and free control structure
const char *lwpng_encoded(lwpng_encode_t*,size_t *len,uint8_t **data);

#endif
