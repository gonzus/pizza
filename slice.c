#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "log.h"
#include "slice.h"

slice SLICE_NULL = { .ptr = 0, .len = 0 };

static void slice_dump_file(slice s, FILE* fp);

int slice_is_null(slice s) {
    return s.ptr == 0;
}

int slice_is_empty(slice s) {
    return s.ptr != 0 && s.len == 0;
}

int slice_get_length(slice s) {
    return s.len;
}

slice slice_wrap_ptr_len(const Byte* ptr, unsigned int len) {
    slice s = { .ptr = ptr, .len = len };
    return s;
}

slice slice_wrap_string(const char* string) {
    return slice_wrap_ptr_len((const Byte*) string, string == 0 ? 0 : strlen(string));
}

slice slice_wrap_string_length(const char* string, unsigned int length) {
    return slice_wrap_ptr_len((const Byte*) string, length);
}

unsigned int slice_to_string(slice s, char* string) {
    memcpy(string, s.ptr, s.len);
    string[s.len] = '\0';
    return s.len;
}

void slice_dump(slice s) {
    slice_dump_file(s, stderr);
}

int slice_tokenize(slice s, slice sep, slice* token) {
    unsigned int start = 0;
    if (token->ptr) {
        start = (token->ptr - s.ptr) + token->len + 1;
        token->ptr = 0;
    }

    if (start >= s.len) {
        return 0;
    }

    const Byte* p = s.ptr + start;
    unsigned int l = s.len - start;
    unsigned int j = 0;
    unsigned int k = 0;
    int found = 0;
    for (j = 0; !found && j < l; ++j) {
        for (k = 0; !found && k < sep.len; ++k) {
            if (p[j] == sep.ptr[k]) {
                found = 1;
            }
        }
    }

    token->ptr = p;
    if (found) {
        token->len = j - 1;
    } else {
        token->len = l;
    }
    return 1;
}


static void slice_dump_file(slice s, FILE* fp) {
    char byte[16*3+1];
    int bpos = 0;
    char dump[16+1];
    int dpos = 0;
    unsigned int row = 0;
    unsigned int col = 0;
    for (unsigned int j = 0; j < s.len; ++j) {
        unsigned char uc = (unsigned char) s.ptr[j];
        unsigned int ui = (unsigned int) uc;
        bpos += sprintf(byte + bpos, " %02x", ui);
        dpos += sprintf(dump + dpos, "%c", isprint(uc) ? uc : '.');
        col++;
        if (col == 16) {
            fprintf(fp, "%06x | %s | %s\n", row, byte, dump);
            col = bpos = dpos = 0;
            row += 16;
        }
    }
    if (dpos > 0) {
        fprintf(fp, "%06x | %s%*c | %s\n", row, byte, (16-col)*3, ' ', dump);
    }
}
