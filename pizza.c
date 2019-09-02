#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pizza.h"

#define BUFFER_MIN_CAP 32
#define BUFFER_GROWTH_FACTOR 2

static void buffer_ensure_extra(buffer* b, unsigned int extra);
static void buffer_ensure_total(buffer* b, unsigned int total);

slice slice_wrap_ptr(const char* p) {
    return slice_wrap_ptr_len(p, p == 0 ? 0 : strlen(p));
}

slice slice_wrap_ptr_len(const char* p, unsigned int l) {
    slice s;
    s.ptr = p;
    s.len = l;
    return s;
}

buffer* buffer_build(void) {
    buffer* b = (buffer*) calloc(1, sizeof(buffer));
    return b;
}

void buffer_destroy(buffer* b) {
    free(b->ptr);
    free(b);
}

unsigned int buffer_length(const buffer* b) {
    return b->pos;
}

buffer* buffer_set_character(buffer* b, char c) {
    buffer_ensure_total(b, 1);
#if 0
    unsigned char uc = (unsigned char) c;
    unsigned int ui = (unsigned int) uc;
    fprintf(stderr, "COPYC [0x%02x]\n", ui);
#endif
    b->pos = 0;
    b->ptr[b->pos++] = c;
    return b;
}

buffer* buffer_set_string(buffer* b, const char* s) {
    unsigned int len = s == 0 ? 0 : strlen(s);
    buffer_ensure_total(b, len);
#if 0
    fprintf(stderr, "COPYS [%u] [%s]\n", len, s);
#endif
    memcpy(b->ptr, s, len);
    b->pos = len;
    return b;
}

buffer* buffer_append_character(buffer* b, char c) {
    buffer_ensure_extra(b, 1);
#if 0
    unsigned char uc = (unsigned char) c;
    unsigned int ui = (unsigned int) uc;
    fprintf(stderr, "APPENDC [0x%02x]\n", ui);
#endif
    b->ptr[b->pos++] = c;
    return b;
}

buffer* buffer_append_string(buffer* b, const char* s) {
    unsigned int len = s == 0 ? 0 : strlen(s);
    buffer_ensure_extra(b, len);
#if 0
    fprintf(stderr, "APPENDS [%u] [%s]\n", len, s);
#endif
    memcpy(b->ptr + b->pos, s, len);
    b->pos += len;
    return b;
}

buffer* slice_to_buffer(const slice s) {
    buffer* b = buffer_build();
    buffer_ensure_extra(b, s.len);
    memcpy(b->ptr, s.ptr, s.len);
    b->pos = s.len;
    return b;
}

slice buffer_to_slice(const buffer* b) {
    return slice_wrap_ptr_len(b->ptr, b->pos);
}

void slice_dump(const slice s) {
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
            fprintf(stderr, "%06x | %s | %s\n", row, byte, dump);
            col = bpos = dpos = 0;
            row += 16;
        }
    }
    if (dpos > 0) {
        fprintf(stderr, "%06x | %s%*c | %s\n", row, byte, (16-col)*3, ' ', dump);
    }
}

const char* slice_to_string(const slice s, char* string) {
    sprintf(string, "%*.*s", s.len, s.len, s.ptr);
    return string;
}

static void buffer_ensure_extra(buffer* b, unsigned int extra) {
    buffer_ensure_total(b, extra + b->pos);
}

static void buffer_ensure_total(buffer* b, unsigned int total) {
    int changes = 0;
    unsigned int current = b->cap;
    while (total > current) {
        ++changes;
        unsigned int next = current == 0 ? BUFFER_MIN_CAP : current * BUFFER_GROWTH_FACTOR;
        fprintf(stderr, "ENSURE %u -> %d\n", current, next);
        current = next;
    }
    if (changes) {
        b->cap = current;
        b->ptr = realloc((void*) b->ptr, b->cap);
    }
}
