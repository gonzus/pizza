#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "buffer.h"

#define BUFFER_DEFAULT_CAPACITY 32  // default size for buffer
#define BUFFER_GROWTH_FACTOR     2  // factor to make buffer grow when needed

static void buffer_ensure_extra(buffer* b, unsigned int extra);
static void buffer_ensure_total(buffer* b, unsigned int total);
static void buffer_realloc(buffer* b, unsigned int cap);

buffer* buffer_build(void) {
    buffer* b = (buffer*) calloc(1, sizeof(buffer));
    return b;
}

buffer* buffer_clone(const buffer* b) {
    buffer* n = buffer_build();
    slice s = buffer_get_slice(b);
    buffer_set_slice(n, s);
    return n;
}

void buffer_destroy(buffer* b) {
    free(b->ptr);
    free(b);
}

unsigned int buffer_pack(buffer* b) {
    if (b->pos < b->cap) {
        buffer_realloc(b, b->pos);
    }
    return b->pos;
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
    LOG_INFO("SETB [0x%02x:%c]", (unsigned int) u, isprint(u) ? u : '.');
    b->pos = 0;
    b->ptr[b->pos++] = u;
    return b;
}

buffer* buffer_set_slice(buffer* b, slice s) {
    buffer_ensure_total(b, s.len);
    LOG_INFO("SETS [%u:%.*s]", s.len, s.len, s.ptr);
    memcpy(b->ptr, s.ptr, s.len);
    b->pos = s.len;
    return b;
}

buffer* buffer_append_byte(buffer* b, Byte u) {
    buffer_ensure_extra(b, 1);
    LOG_INFO("APPENDB [0x%02x:%c]", (unsigned int) u, isprint(u) ? u : '.');
    b->ptr[b->pos++] = u;
    return b;
}

buffer* buffer_append_slice(buffer* b, slice s) {
    buffer_ensure_extra(b, s.len);
    LOG_INFO("APPENDS [%u:%.*s]", s.len, s.len, s.ptr);
    memcpy(b->ptr + b->pos, s.ptr, s.len);
    b->pos += s.len;
    return b;
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
        LOG_INFO("ENSURE %u -> %u", current, next);
        current = next;
    }
    if (changes) {
        buffer_realloc(b, current);
    }
    assert(b->cap >= total);
}

static void buffer_realloc(buffer* b, unsigned int cap) {
    LOG_INFO("REALLOC %p: %u to %u bytes", b->ptr, b->cap, cap);
    Byte* tmp = realloc((void*) b->ptr, cap);
    if (!tmp) {
        LOG_ERROR("Out of memory when reallocating %p from %u to %u bytes", b->ptr, b->cap, cap);
    }
    b->ptr = tmp;
    b->cap = cap;
    return;
}
