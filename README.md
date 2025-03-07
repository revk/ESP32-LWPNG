# PNG encoder/decoder for ESP32

Config (`Kconfig`) determins if encoder and/or decoder are included.

# Decoder

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

`lwpng_decode_t *lwpng_decode(void *opaque,lwpng_cb_start_t*,lwpng_cb_pixel_t*,alloc_func,free_func,void *allocopaque)`

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

`const char *lwpng_data(lwpng_decode_t*,size_t len,uint8_t *data)`

Call this repeatedly, or all in one go if you prefer, with bytes from the PNG in order. Returns NULL if OK, else an error string. Can be used one byte at a time if you wish, but blocks are likely to be more efficient.

Note that once an error happens, all further calls do nothing and return the latched error, so you can leave checking error until the end, and check it at `lwpng_end` call.

## Finish

`const char *lwpng_decoded(lwpng_decode_t**)`

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

# Encoder

This is used to generate a PNG in memory (using `open_memstream`)

## Overview

Simple in memory PNG generator - no filter logic. The error logic is the same as encoding.

## Init

`lwpng_encode_t *lwpng_encode_1bit(uint32_t w,uint32_t h,alloc_func,free_func,void *allocopaque)`

The alloc/free/allocopaque are used by `zlib` (see decoder). `w` is width (pixels) and `h` is height (pixels). This encoded 1 bit per pixel with black/white palette.

Other init functions may be added for other encodings.

## Data

`const char *lwpng_encode_scanline(lwpng_encode_t*,uint8_t *data)`

This is called per scan line with a full scan line (according to PNG encopding, but with no filter byte prefix). Should be called for every scan line in order.

## Finish

`const char *lwpng_encoded(lwpng_encode_t**,size_t *len,uint8_t **data)`

Fills in `data` and `len` for the encoded png image, else  NULL/0 and error return string.

# TinyPNG

TinyPNG is a format I am considering adding to both encode and decode, and a recode function. The logic is that a normal PNG file includes :-

- 12 byte header
- 4 byte per chunk length
- 4 byte per chunk ID
- 4 byte per chunk CRC
- 13 bytes of `IHDR` data which could be reduced to 5 easily
- Any number of additional chunks apart from `IHDR`, `PLTE`, `tRNS` and `IDAT`
- Possible multipler concatenated `IDAT` fields each with header and CRC

Even an optimal PNG with `PLTE` and `tRNS` will always have 58 more bytes than needed.

So the idea is that small icons, and images, maybe even fonts, could use a more compact PNG format internally in memory. Fonts would be a really good example of savings if there are 96 characters all wasting 58 bytes for example.

The format is as follows and assumes the total length is known. No CRCs included.

|Field|Size|Meaning|
|-----|----|-------|
|`w`|2 bytes (network byte order)|Width (this format does not allow more than 65535 pixels wide)|
|`h`|2 bytes (network byte order)|Height (this format does not allow more than 65535 pixels hight)|
|`type`|1 byte|Low 3 bits are PNG `color` and top 5 bits are PNG `depth`|
|`plte`|1 byte N, plus N times 3 bytes|Present if `type` bit 0 (PLTE) set.|
|`trns`|1 byte N, plus N bytes|Present for `color` 3, 2, or 0. The `tRNS` bytes, up to 256 bytes for palette `tRNS`.|
|`idat`|To end|The IDAT content in one block|

Note the number of bytes have special case for 0, as follows:

- `PLTE` starts with 1 byte number of entries, then each entry is 3 bytes (R/G/B). Special case of length 0 means 256 entries.
- `tRNS` starts with 1 byte length as number of bytes. The special case is `color` 3 and `depth` 8 where a length of 0 means 256 bytes, otherwise length 0 means no `tRNS`. In the event of a `color` 3 and `depth` 8 where no `tRNS` is present, a length 1 and value 0xFF is used - meaning palette 0 is opaque (as is the rest of the palette).

## Encode (to be documented)

The initial call to encode will say it is this format, and create a TinyPNG data buffer instead of a full PNG.

## Decode (to be documented)

This initial call to decode will say it is this format, with the same call backs for plotting. The only restriction may be that the first block should cover the header and not be byte by byte perhaps. Though it may be that this is coded to assume the whole file is in memory (after all that is the point) and so one call with ploting callback and whole TinyPNG perhaps.

## Recode (to be documented)

This will work like the decode, but instead of the decode callbacks a TinyPNG file is generated in memory and the final call provides that data/len.

This means the decode could be read from file or stream part by part to save space. This will also not invoke `zlib` so will be very space efficient in making the TinyPNG file in memory for later use.
