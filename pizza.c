#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "log.h"
#include "pizza.h"

#define BUFFER_DEFAULT_CAPACITY 32  // default size for buffer
#define BUFFER_GROWTH_FACTOR     2  // factor to make buffer grow when needed

slice SLICE_NULL = { .ptr = 0, .len = 0 };

static void slice_dump_file(slice s, FILE* fp);
static void buffer_ensure_extra(buffer* b, unsigned int extra);
static void buffer_ensure_total(buffer* b, unsigned int total);

int slice_is_null(slice s) {
    return s.ptr == 0;
}

int slice_is_empty(slice s) {
    return s.ptr != 0 && s.len == 0;
}

int slice_get_length(slice s) {
    return s.len;
}

static slice slice_wrap_ptr_len(const Byte* ptr, unsigned int len) {
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


buffer* buffer_build(void) {
    buffer* b = (buffer*) calloc(1, sizeof(buffer));
    return b;
}

void buffer_destroy(buffer* b) {
    free(b->ptr);
    free(b);
}

slice buffer_get_slice(const buffer* b) {
    return slice_wrap_ptr_len(b->ptr, b->pos);
}

unsigned int buffer_length(const buffer* b) {
    return b->pos;
}

unsigned int buffer_capacity(const buffer* b) {
    return b->cap;
}

void buffer_clear(buffer* b) {
    b->pos = 0;
}

buffer* buffer_set_byte(buffer* b, Byte u) {
    buffer_ensure_total(b, 1);
    LOG("COPYC [0x%02x]\n", (unsigned int) u);
    b->pos = 0;
    b->ptr[b->pos++] = u;
    return b;
}

buffer* buffer_set_slice(buffer* b, slice s) {
    buffer_ensure_total(b, s.len);
    LOG("COPYS [%u] [%.*s]\n", s.len, s.len, s.ptr);
    memcpy(b->ptr, s.ptr, s.len);
    b->pos = s.len;
    return b;
}

buffer* buffer_append_byte(buffer* b, Byte u) {
    buffer_ensure_extra(b, 1);
    LOG("APPENDC [0x%02x]\n", (unsigned int) u);
    b->ptr[b->pos++] = u;
    return b;
}

buffer* buffer_append_slice(buffer* b, slice s) {
    buffer_ensure_extra(b, s.len);
    LOG("APPENDS [%u] [%.*s]\n", s.len, s.len, s.ptr);
    memcpy(b->ptr + b->pos, s.ptr, s.len);
    b->pos += s.len;
    return b;
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

static void buffer_ensure_extra(buffer* b, unsigned int extra) {
    buffer_ensure_total(b, extra + b->pos);
}

static void buffer_ensure_total(buffer* b, unsigned int total) {
    int changes = 0;
    unsigned int current = b->cap;
    while (total > current) {
        ++changes;
        unsigned int next = current == 0 ? BUFFER_DEFAULT_CAPACITY : current * BUFFER_GROWTH_FACTOR;
        LOG("ENSURE %u -> %u\n", current, next);
        current = next;
    }
    if (changes) {
        b->cap = current;
        b->ptr = realloc((void*) b->ptr, b->cap);
    }
    assert(b->cap >= total);
}
