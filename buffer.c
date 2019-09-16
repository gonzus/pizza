#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
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

void buffer_destroy(buffer* b) {
    free(b->ptr);
    free(b);
}

unsigned int buffer_length(const buffer* b) {
    return b->pos;
}

unsigned int buffer_capacity(const buffer* b) {
    return b->cap;
}

slice buffer_get_slice(const buffer* b) {
    return slice_wrap_ptr_len(b->ptr, b->pos);
}

buffer* buffer_clone(const buffer* b) {
    slice s = buffer_get_slice(b);
    buffer* n = buffer_build();
    buffer_append_slice(n, s);
    return n;
}

void buffer_pack(buffer* b) {
    if (b->pos < b->cap) {
        buffer_realloc(b, b->pos);
    }
}

void buffer_clear(buffer* b) {
    b->pos = 0;
}

void buffer_append_byte(buffer* b, Byte u) {
    buffer_ensure_extra(b, 1);
    LOG_INFO("APPENDB [0x%02x:%c]", (unsigned int) u, isprint(u) ? u : '.');
    b->ptr[b->pos++] = u;
}

void buffer_append_slice(buffer* b, slice s) {
    buffer_ensure_extra(b, s.len);
    LOG_INFO("APPENDS [%u:%.*s]", s.len, s.len, s.ptr);
    memcpy(b->ptr + b->pos, s.ptr, s.len);
    b->pos += s.len;
}

void buffer_format_signed(buffer* b, long long l) {
    char cstr[99];
    int clen = sprintf(cstr, "%lld", l);
    buffer_append_slice(b, slice_wrap_string_length(cstr, clen));
}

void buffer_format_unsigned(buffer* b, unsigned long long l) {
    char cstr[99];
    int clen = sprintf(cstr, "%llu", l);
    buffer_append_slice(b, slice_wrap_string_length(cstr, clen));
}

void buffer_format_double(buffer* b, double d) {
    char cstr[99];
    int clen = sprintf(cstr, "%f", d);
    buffer_append_slice(b, slice_wrap_string_length(cstr, clen));
}

void buffer_format(buffer* b, const char* fmt, ...) {
    va_list ap1;
    va_list ap2;

    va_start(ap1, fmt);
    va_copy(ap2, ap1);

    int size = vsnprintf(0, 0, fmt, ap1);
    va_end(ap1);
    LOG_INFO("FORMAT [%s] => %d bytes", fmt, size);

    buffer_ensure_extra(b, size + 1);  // snprintf below will also include a '\0'

    vsnprintf((char*) b->ptr + b->pos, size + 1, fmt, ap2);
    va_end(ap2);
    LOG_INFO("FORMATTED [%d:%.*s]", size, size, b->ptr + b->pos);

    b->pos += size;
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
        LOG_DEBUG("ENSURE %u -> %u", current, next);
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
