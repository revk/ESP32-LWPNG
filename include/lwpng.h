// PNG expander

#ifdef	CONFIG_IDF_TARGET
#include <miniz.h>
#define	alloc_int	size_t
#else
// Non ESP
#include <stdint.h>
#include <zlib.h>
#define	alloc_int	uInt
#endif

typedef	struct	lwpng_s	lwpng_t; // This is the internal (malloc'd) structure for png decoding

// Callbacks - return NULL for OK, else return an error string

// Called at start of first IDAT
typedef const char *lwpng_cb_start_t(void *opaque,uint32_t w,uint32_t h,uint8_t hasalpha);

// Called for each pixel, values all scaled to uint16_t, no gamma/etc adjustment
// Note the order may not be sequential if interlaced, so should not be assumed
typedef	const char *lwpng_cb_pixel_t(void *opaque,uint32_t x,uint32_t y,uint16_t r,uint16_t g,uint16_t b,uint16_t a);

// Calls to decode PNG
// Note, once an error happens it latches and all further lwpng_data calls are ignored, as such you can just check error on lwpng_end

// Allocate a new PNG decode, alloc/free can be NULL for system defaults
lwpng_t *lwpng_init(void *opaque,lwpng_cb_start_t*,lwpng_cb_pixel_t*,alloc_func,free_func,void *allocopaque);

// Process data sequentially as data received for PNG file, returns NULL if OK, else error string
const char *lwpng_data(lwpng_t*,size_t len,uint8_t *data);

// End processing, frees the control structure, returns NULL if OK, else error string
const char *lwpng_end(lwpng_t**);
