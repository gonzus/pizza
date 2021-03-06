#ifndef UTF8_H_
#define UTF8_H_

/*
 * UTF8 encoding and decoding, using Slice and Buffer.
 */

#include "slice.h"
#include "buffer.h"

// Value that indicates an invalid Unicode Rune.
#define UTF8_INVALID_RUNE ((Rune) -1)

typedef uint32_t Rune;  // type for Unicode Runes

// Decode the bytes in a Slice into a valid Unicode Rune.
// Change the Slice to point to the still undecoded portion.
Rune utf8_decode(Slice* s);

// Encode a Unicode Rune into a Buffer (therefore, containing Bytes).
// Return the number of bytes the Unicode Rune was encoded into.
// If the Rune was not valid Unicode, return 0.
unsigned int utf8_encode(Rune r, Buffer* b);

#endif
