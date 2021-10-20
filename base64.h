#ifndef BASE64_H_
#define BASE64_H_

/*
 * base64 encoding and decoding using Slice and Buffer
 */

#include "buffer.h"

int base64_decode(Slice encoded, Buffer *decoded);
int base64_encode(Slice decoded, Buffer *encoded);

#endif
