#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

static void dump_line(FILE* fp, int row, const char* byte, int white, const char* text) {
    fprintf(fp, "%06x | %s%*s | %-16s |\n", row, byte, white, "", text);
}

void dump_bytes(FILE* fp, const void* ptr, size_t len) {
    fprintf(fp, "bytes at %p, len %lu\n", ptr, len);
    char byte[16*3+1];
    int bpos = 0;
    char text[16+1];
    int dpos = 0;
    int row = 0;
    int col = 0;
    for (size_t j = 0; j < len; ++j) {
        unsigned char uc = ((unsigned char*) ptr)[j];
        unsigned int ui = (unsigned int) uc;
        bpos += sprintf(byte + bpos, "%s%02x", col ? " " : "", ui);
        dpos += sprintf(text + dpos, "%c", uc <= 0x7f && isprint(uc) ? uc : '.');
        col++;
        if (col == 16) {
            dump_line(fp, row, byte, 0, text);
            col = bpos = dpos = 0;
            row += 16;
        }
    }
    if (dpos > 0) {
        dump_line(fp, row, byte, (16-col)*3, text);
    }
}

void* memory_realloc(void* ptr, size_t len) {
    if (len <= 0) {
        // want to release the memory
        free(ptr);
        return 0;
    }

    // want to create / resize the memory
    void* tmp = (void*) realloc(ptr, len);
    if (tmp) {
        return tmp;
    }

    // bad things happened
    if (errno) {
        fprintf(stderr, "Error %d (%s) calling realloc(%p, %lu)",
                errno, strerror(errno), ptr, len);
    } else {
        fprintf(stderr, "Could not allocate memory calling realloc(%p, %lu)\n",
                ptr, len);
    }
    abort();
    return 0;
}
