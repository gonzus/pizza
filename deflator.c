#include <string.h>
#include <stdlib.h>
#include <zlib.h>
#include "deflator.h"

#define ZLIB_CHUNK 16384
#define ZLIB_LEVEL 6

void deflator_build(Deflator* deflator, int chunk_size) {
    memset(deflator, 0, sizeof(Deflator));
    deflator->chunk_size = chunk_size <= 0 ?  ZLIB_CHUNK : chunk_size;
    deflator->chunk = malloc(deflator->chunk_size);
}

void deflator_destroy(Deflator* deflator) {
    free((void*) deflator->chunk);
}

int deflator_uncompress(Deflator* deflator, Slice compressed, Buffer* uncompressed) {
    int bad = 0;
    int ret = Z_OK;
    int zinited = 0;
    z_stream strm;

    do {
        // allocate inflate state
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        ret = inflateInit(&strm);
        if (ret != Z_OK) {
            break;
        }
        zinited = 1;

        // decompress until deflate stream ends or end of input
        int pos = 0;
        do {
            int flush = 0;
            uint32_t left = compressed.len - pos;
            if (left < deflator->chunk_size) {
                strm.avail_in = left;
                flush = Z_FINISH;
            } else {
                strm.avail_in = deflator->chunk_size;
                flush = Z_NO_FLUSH;
            }
            strm.next_in = (Bytef *) compressed.ptr + pos;
            pos += strm.avail_in;

            // run inflate() on input until output buffer not full
            do {
                strm.avail_out = deflator->chunk_size;
                strm.next_out = deflator->chunk;
                ret = inflate(&strm, flush);    // no bad return value
                assert(ret != Z_STREAM_ERROR);  // state not clobbered
                switch (ret) {
                    case Z_NEED_DICT:
                    case Z_DATA_ERROR:
                    case Z_MEM_ERROR:
                        bad = 1;
                        break;
                }
                if (!bad) {
                    unsigned have = deflator->chunk_size - strm.avail_out;
                    buffer_append_slice(uncompressed, slice_build_from_ptr_len((const char*) deflator->chunk, have));
                }
            } while (!bad && strm.avail_out == 0);
        } while (!bad && ret != Z_STREAM_END); // done when inflate() says it's done
    } while (0);

    if (zinited) {
        // deallocate inflate state
        (void)inflateEnd(&strm);
        zinited = 0;
    }

    /* clean up and return */
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

int deflator_compress(Deflator* deflator, Slice uncompressed, Buffer* compressed, int level) {
    int ret = Z_OK;
    z_stream strm;
    int zinited = 0;

    if (level <= 0) {
        level = ZLIB_LEVEL;
    }
    do {
        int flush = 0;
        // allocate deflate state
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        ret = deflateInit(&strm, level);
        if (ret != Z_OK) {
            break;
        }
        zinited = 1;

        int pos = 0;
        do {
            uint32_t left = uncompressed.len - pos;
            if (left < deflator->chunk_size) {
                strm.avail_in = left;
                flush = Z_FINISH;
            } else {
                strm.avail_in = deflator->chunk_size;
                flush = Z_NO_FLUSH;
            }
            strm.next_in = (Bytef *) uncompressed.ptr + pos;
            pos += strm.avail_in;

            // run deflate() on input until output buffer not full,
            // finish compression if all of uncompressed has been read in
            do {
                strm.avail_out = deflator->chunk_size;
                strm.next_out = deflator->chunk;
                ret = deflate(&strm, flush);    // no bad return value
                assert(ret != Z_STREAM_ERROR);  // state not clobbered
                unsigned have = deflator->chunk_size - strm.avail_out;
                buffer_append_slice(compressed, slice_build_from_ptr_len((const char*) deflator->chunk, have));
            } while (strm.avail_out == 0);
            assert(strm.avail_in == 0); // all input will be used

        } while (flush != Z_FINISH);  // done when last data in file processed
        assert(ret == Z_STREAM_END);  // stream will be complete
    } while (0);

    if (zinited) {
        // deallocate deflate state
        (void)deflateEnd(&strm);
        zinited = 0;
    }
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
