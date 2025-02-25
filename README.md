# PNG expander for ESP32

Expand PNG image file, from memory or streamed/block-a-time, to pixel plots.

## Overview

This is designed to be lightweight (in memory footprint) - sadly the footprint for the inflate is around 50kB regardless, but the system allows the original PNG to be streamed in to the decoder. Use of custom allocation functions can allow use of SPI RAM if available.

The decoder calls a function for each pixel rather than expanding in to a memory buffer. This is mainly designed for e-paper where, for example, the bit map is 1 bit per pixel, so 480x800 is 48kB. The PNG expander can be used streaming a PNG from SD card or HTTP or some such, plotting pixels directly into the frame buffer for the e-paper.

This is fully functional, including all PNG formats and interlacing, grey/colour/indexed/alpha. Colour values are not adjusted for gamma, etc.

### Memory usage

The main memory usage is from *zlib*, which typically needs 32kB back buffer and more for Huffman tables. Tests suggest around 50kB total. The PNG decoder uses memory for a scan line (so width pixels, which may be as much as 8 bytes each), and for the `PLTE` and `tRNS` chunks. There is a tradeoff, when `PLTE` is used the scan line pixels are max 1 byte per pixel, not 8. When `tRNS` is used the scan lines do not have *alpha* so max 6 bytes per pixel.

If memory is short it is worth saving PNG using 1 bit indexed for e-paper usage anyway, as this will reduce scan line storage as well as size of PNG if stored. But any PNG format is accepted.

### Code size

You need the managed component `zlib`, i.e. `idf.py add-dependency zlib`.

The code is reasonably compact, but there is a config option for *full checks*. When disabled (default) it is assumed the PNG is a valid file. Checks are done for the initial signature, and that the file reaches the end, but most other checks are not done. This could cause some unexpected results if the file is not in fact valid, but should not crash.

With *checks* enabled, the file validity is checked at many stages, including chunk CRC checks and value range checks, creating a sensible error string if not valid. This takes more code space.

## Init

`lwpng_t *lwpng_init(void *opaque,lwpng_cb_start_t*,lwpng_cb_pixel_t*,alloc_func,free_func,void *allocopaque)`

This allocates a control structure, returning NULL if failed. The control structure is used in the following functions. The only possible failure would be a memory allocation failure for the control structure.

|Parameter|Meaning|
|---------|-------|
|`opaque`|Passed to the *callback* functions (*start* and *pixel*), so can be NULL|
|`start`|The callback for start of decode, optional (can be NULL)|
|`pixel`|The callback for each pixel, optional (can be NULL)|
|`alloc`|The function for memory allocation, NULL for system default `malloc` (see *zlib*)|
|`free`|The function for memory free, NULL for system default `free` (see *zlib*)|
|`allocopaque`|Passed to *alloc* and *free* functions, so can be NULL|

## Feeding data

`const char *lwpng_data(lwpng_t*,size_t len,uint8_t *data)`

Call this repeatedly, or all in one go if you prefer, with bytes from the PNG in order. Returns NULL if OK, else an error string. Can be used one byte at a time if you wish, but blocks are likely to be more effectient.

Note that once an error happens, all further calls do nothing and return the latched error, so you can leave checking error until the end, and check it at `lwpng_end` call.

## Finish

`const char *lwpng_end(lwpng_t**)`

This frees the control structure and also NULLs the value. Returns NULL if OK, else an error string. You have to call this even if a previous error is returned, so as to free space.

## Callback functions

### Start

`const char *lwpng_cb_start_t(void *opaque,uint32_t w,uint32_t h,uint8_t hasalpha)`

Called once `IDAT` is encountered with *width*, *height* and a *hasalpha* flag for alpha channel used. Typically this is where the caller may allocate space to store the image data, etc.

Returning a string indicates an error. Return NULL for OK.

### Pixel

`const char *lwpng_cb_pixel_t(void *opaque,uint32_t x,uint32_t y,uint16_t r,uint16_t g,uint16_t b,uint16_t a)`

Called for each pixel. Again, the return is NULL if OK, else error string.

The values `r`, `g`, `b`, and `a` (for *alpha*) are scaled to 16 bits regardless or original encoding in PNG file, i.e. values `0` to `65535`. Alpha is opacity, so `65535` is fully opaque and `0` fully transparrent.

The range is always `0` to `65535`, e.g. if original is 1 bit per pixel, you see `0` or `65535`. For 2 bit per pixel you see `0`, `21845`, `43690`, `65535`. For 8 bit coding, the 256 values you see are `0`, `257`, `514`, ... `65278`, `65535`.

Note: Pixels are not called in order if *interlace* is in use, so do not assume they will be.

### Alloc/free

See `zlib` for details. You may need a simple wrapper function to call normal `malloc`/`free`.
