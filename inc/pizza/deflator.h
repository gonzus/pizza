#ifndef DEFLATE_H_
#define DEFLATE_H_

#include "buffer.h"

typedef struct Deflator {
    uint32_t chunk_size;
    unsigned char* chunk;
} Deflator;

void deflator_build(Deflator* deflator, int chunk_size);
void deflator_destroy(Deflator* deflator);

int deflator_compress(Deflator* deflator, Slice uncompressed, Buffer* compressed, int level);
int deflator_uncompress(Deflator* deflator, Slice compressed, Buffer* uncompressed);

#endif
