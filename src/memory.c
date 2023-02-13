#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include "stb_sprintf.h"
#include "console.h"
#include "log.h"
#include "memory.h"

static void dump_line(int row, const char* byte, int white, const char* text) {
    console_printf("%06x | %s%*s | %-16s |\n", row, byte, white, "", text);
}

void dump_bytes(const void* ptr, size_t len) {
    console_printf("bytes at %p, len %lu\n", ptr, len);
    char byte[16*3+1];
    int bpos = 0;
    char text[16+1];
    int dpos = 0;
    int row = 0;
    int col = 0;
    unsigned char* ucp = (unsigned char*) ptr;
    for (size_t j = 0; j < len; ++j) {
        unsigned char uc = ucp[j];
        unsigned int ui = (unsigned int) uc;
        bpos += stbsp_sprintf(byte + bpos, "%s%02x", col ? " " : "", ui);
        dpos += stbsp_sprintf(text + dpos, "%c", uc <= 0x7f && isprint(uc) ? uc : '.');
        col++;
        if (col == 16) {
            dump_line(row, byte, 0, text);
            col = bpos = dpos = 0;
            row += 16;
        }
    }
    if (dpos > 0) {
        dump_line(row, byte, (16-col)*3, text);
    }
}

void* memory_realloc(void* ptr, size_t len) {
    if (len <= 0) {
        // want to release the memory
        if (ptr) {
            free(ptr);
        }
        return 0;
    }

    // want to create / resize the memory
    void* tmp = (void*) realloc(ptr, len);
    if (tmp) {
        return tmp;
    }

    // bad things happened
    if (errno) {
        LOG_WARN("Could not realloc %p to %lu bytes", ptr, len);
    } else {
        LOG_WARN("Could not allocate memory to realloc %p to %lu bytes", ptr, len);
    }
    abort();
    return 0;
}
