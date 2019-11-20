#ifndef UTF8_H_
#define UTF8_H_

#include "slice.h"
#include "buffer.h"

#define UTF8_INVALID_RUNE ((Rune) -1)

typedef uint32_t Rune;

/*
 * Decode the bytes in a slice into a valid Unicode rune.
 * If rest is not null, leave there the slice pointing to the rest of the
 * original slice
 */
Rune utf8_decode(Slice s, Slice* rest);

/*
 * Encode a Unicode rune into a buffer of bytes.
 * Return the number of bytes the Unicode rune was encoded into.
 * If the rune was not valid Unicode, return 0.
 */
Byte utf8_encode(Rune r, Buffer* b);

#endif
