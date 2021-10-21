#ifndef MD5_H_
#define MD5_H_

#include <stdint.h>
#include "buffer.h"

#define MD5_DIGEST_LEN 16

typedef struct MD5 {
  uint32_t bits[2];               // number of _bits_ handled mod 2^64
  uint32_t work[4];               // work buffer
  uint8_t input[64];              // input buffer
  uint8_t digest[MD5_DIGEST_LEN]; // actual digest after md5_finalize()
} MD5;

// reset MD5 to start a new computation
void md5_reset(MD5* md5);

// add data from the Slice into MD5
// can be called multiple times to keep adding data
void md5_update(MD5* md5, Slice s);

// finalize computation, leaving (binary) digest in md5->digest
void md5_finalize(MD5* md5);

// format md5->digest as hex characters into Buffer
void md5_format(MD5* md5, Buffer* b);

// compute hash for a single Slice and format it into Buffer, all in one go
void md5_compute(MD5* md5, Slice s, Buffer* b);

#endif
