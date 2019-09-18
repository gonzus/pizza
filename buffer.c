#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "buffer.h"

#define BUFFER_DEFAULT_CAPACITY 32  // default size for Buffer
#define BUFFER_GROWTH_FACTOR     2  // factor to make Buffer grow when needed

#define BUFFER_FLAG_SET(b, f) do { (b)->flg |= ( f); } while (0)
#define BUFFER_FLAG_CLR(b, f) do { (b)->flg &= (~f); } while (0)
#define BUFFER_FLAG_CHK(b, f) ( (b)->flg & (f) )

static void buffer_ensure_extra(Buffer* b, Size extra);
static void buffer_ensure_total(Buffer* b, Size total);
static void buffer_realloc(Buffer* b, Size cap);

Buffer* buffer_build(void) {
    Buffer* b = (Buffer*) malloc(sizeof(Buffer));
    buffer_init(b);
    BUFFER_FLAG_SET(b, BUFFER_FLAG_BUF_IN_HEAP);
    return b;
}

void buffer_init(Buffer* b) {
    memset(b, 0, sizeof(Buffer));
    b->ptr = b->buf;
    b->cap = BUFFER_DATA_SIZE;
}

void buffer_destroy(Buffer* b) {
    if (BUFFER_FLAG_CHK(b, BUFFER_FLAG_PTR_IN_HEAP)) {
        LOG_INFO("DESTROY deleting heap-allocated ptr with %u bytes", b->cap);
        free(b->ptr);
    }
    if (BUFFER_FLAG_CHK(b, BUFFER_FLAG_BUF_IN_HEAP)) {
        LOG_INFO("DESTROY deleting heap-allocated buf");
        free(b);
    }
}

Size buffer_length(const Buffer* b) {
    return b->pos;
}

Size buffer_capacity(const Buffer* b) {
    return b->cap;
}

Slice buffer_get_slice(const Buffer* b) {
    return slice_wrap_ptr_len(b->ptr, b->pos);
}

Buffer* buffer_clone(const Buffer* b) {
    Slice s = buffer_get_slice(b);
    Buffer* n = buffer_build();
    buffer_append_slice(n, s);
    return n;
}

void buffer_pack(Buffer* b) {
    if (!BUFFER_FLAG_CHK(b, BUFFER_FLAG_PTR_IN_HEAP)) {
        return;
    }

    if (b->pos < BUFFER_DATA_SIZE) {
        // using heap ptr now
        LOG_INFO("MIGRATED heap [%p] to stack [%p]", b->ptr, b->buf);
        memcpy(b->buf, b->ptr, b->pos);
        buffer_realloc(b, 0);
        b->ptr = b->buf;
        BUFFER_FLAG_CLR(b, BUFFER_FLAG_PTR_IN_HEAP);
    } else {
        buffer_realloc(b, b->pos);
    }
}

void buffer_clear(Buffer* b) {
    b->pos = 0;
}

void buffer_append_byte(Buffer* b, Byte u) {
    buffer_ensure_extra(b, 1);
    LOG_INFO("APPENDB [0x%02x:%c]", (Size) u, isprint(u) ? u : '.');
    b->ptr[b->pos++] = u;
}

void buffer_append_slice(Buffer* b, Slice s) {
    buffer_ensure_extra(b, s.len);
    LOG_INFO("APPENDS [%u:%.*s]", s.len, s.len, s.ptr);
    memcpy(b->ptr + b->pos, s.ptr, s.len);
    b->pos += s.len;
}

void buffer_format_signed(Buffer* b, long long l) {
    char cstr[99];
    Size clen = sprintf(cstr, "%lld", l);
    buffer_append_slice(b, slice_wrap_ptr_len((Byte*) cstr, clen));
}

void buffer_format_unsigned(Buffer* b, unsigned long long l) {
    char cstr[99];
    Size clen = sprintf(cstr, "%llu", l);
    buffer_append_slice(b, slice_wrap_ptr_len((Byte*) cstr, clen));
}

void buffer_format_double(Buffer* b, double d) {
    char cstr[99];
    Size clen = sprintf(cstr, "%f", d);
    buffer_append_slice(b, slice_wrap_ptr_len((Byte*) cstr, clen));
}

void buffer_format(Buffer* b, const char* fmt, ...) {
    va_list ap1;
    va_list ap2;

    va_start(ap1, fmt);
    va_copy(ap2, ap1);

    Size size = vsnprintf(0, 0, fmt, ap1);
    va_end(ap1);
    LOG_INFO("FORMAT [%s] => %d bytes", fmt, size);

    buffer_ensure_extra(b, size + 1);  // vsnprintf below will also include a '\0'

    vsnprintf((char*) b->ptr + b->pos, size + 1, fmt, ap2);
    va_end(ap2);
    LOG_INFO("FORMATTED [%d:%.*s]", size, size, b->ptr + b->pos);

    b->pos += size;
}


static void buffer_ensure_extra(Buffer* b, Size extra) {
    buffer_ensure_total(b, extra + b->pos);
}

static void buffer_ensure_total(Buffer* b, Size total) {
    Size changes = 0;
    Size current = b->cap;
    LOG_INFO("ENSURE currently using %u out of %u", b->pos, b->cap);
    while (total > current) {
        ++changes;
        Size next = current == 0 ? BUFFER_DEFAULT_CAPACITY : current * BUFFER_GROWTH_FACTOR;
        LOG_DEBUG("ENSURE %u -> %u", current, next);
        current = next;
    }
    if (changes) {
        if (BUFFER_FLAG_CHK(b, BUFFER_FLAG_PTR_IN_HEAP)) {
            buffer_realloc(b, current);
        } else {
            // using stack buf now
            Size old = b->cap;
            b->ptr = 0;
            b->cap = 0;
            buffer_realloc(b, current);
            memcpy(b->ptr, b->buf, old);
            LOG_INFO("MIGRATED stack [%u:%p] to heap [%u:%p]", old, b->buf, current, b->ptr);
            BUFFER_FLAG_SET(b, BUFFER_FLAG_PTR_IN_HEAP);
        }
    }
    assert(b->cap >= total);
}

static void buffer_realloc(Buffer* b, Size cap) {
    LOG_INFO("REALLOC %p: %u to %u bytes", b->ptr, b->cap, cap);
    Byte* tmp = realloc((void*) b->ptr, cap);
    if (!tmp) {
        LOG_ERROR("Out of memory when reallocating %p from %u to %u bytes", b->ptr, b->cap, cap);
    }
    b->ptr = tmp;
    b->cap = cap;
    return;
}
