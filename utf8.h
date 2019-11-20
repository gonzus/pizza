#ifndef UTF8_H_
#define UTF8_H_

#include <stdint.h>

typedef uint8_t  Byte;
typedef uint32_t Rune;

Byte* utf8_decode(Byte* buf, Rune* r);
Byte utf8_encode(Byte* buf, Rune r);

#endif
