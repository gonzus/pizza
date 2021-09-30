#ifndef UTF8_H_
#define UTF8_H_

/*
 * UTF8 encoding and decoding, using Slice and Buffer.
 */

#include "slice.h"
#include "buffer.h"

// Value that indicates an invalid Unicode rune.
#define UTF8_INVALID_RUNE ((uint32_t) -1)

// Decode the bytes in a Slice into a valid Unicode rune.
// Change the Slice to point to the still undecoded portion.
uint32_t utf8_decode(Slice* s);

// Encode a Unicode rune into a Buffer (therefore, containing Bytes).
// Return the number of bytes the Unicode rune was encoded into.
// If the rune was not valid Unicode, return 0.
unsigned int utf8_encode(uint32_t r, Buffer* b);

// TODO
// Use https://www.fileformat.info/info/unicode/char/fffd/index.htm
// to indicate errors in UTF-8 decoding

#endif
