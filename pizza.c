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

buffer* buffer_build(unsigned int cap) {
    buffer* b = (buffer*) calloc(1, sizeof(buffer));
    return b;
}

unsigned int buffer_length(const buffer* b) {
    return b->pos;
}

buffer* buffer_append_string(buffer* b, const char* s) {
    unsigned int len = s == 0 ? 0 : strlen(s);
    buffer_ensure_extra(b, len);
    fprintf(stderr, "APPEND [%u] [%s]\n", len, s);
    memcpy(b->ptr + b->pos, s, len);
    b->pos += len;
    return b;
}

buffer* slice_to_buffer(const slice s) {
    buffer* b = buffer_build(s.len);
    buffer_ensure_extra(b, s.len);
    memcpy(b->ptr, s.ptr, s.len);
    b->pos = s.len;
    return b;
}

slice buffer_to_slice(const buffer* b) {
    return slice_wrap_ptr_len(b->ptr, b->pos);
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
        fprintf(stderr, "BUFFER %u -> %d\n", current, next);
        current = next;
    }
    if (changes) {
        b->cap = current;
        b->ptr = realloc((void*) b->ptr, b->cap);
    }
}
