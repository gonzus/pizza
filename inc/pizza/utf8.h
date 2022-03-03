#ifndef UTF8_H_
#define UTF8_H_

/*
 * UTF8 encoding and decoding, using Slice and Buffer.
 */

#include "buffer.h"

// Value that indicates an invalid Unicode rune -- replacement character
#define UTF8_INVALID_RUNE ((uint32_t) 0xfffd)

// Decode the bytes in a Slice into a valid Unicode rune.
// Change the Slice to point to the still undecoded portion.
uint32_t utf8_decode(Slice* s);

// Encode a Unicode rune into a Buffer (therefore, containing Bytes).
// Return the number of bytes the Unicode rune was encoded into.
// If the rune was not valid Unicode, return 0.
unsigned int utf8_encode(uint32_t r, Buffer* b);

#endif
