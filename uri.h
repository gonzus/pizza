#ifndef URI_H_
#define URI_H_

/*
 * URI encoding and decoding using Slice and Buffer
 */

#include "buffer.h"

int uri_decode(Slice encoded, Buffer *decoded);
int uri_encode(Slice decoded, Buffer *encoded);

#endif
