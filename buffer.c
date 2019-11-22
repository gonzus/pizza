#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "buffer.h"

#if !defined(va_copy)
// need this if we want to compile for C89
#define va_copy(d,s) __va_copy(d,s)
#endif

#define BUFFER_DEFAULT_CAPACITY BUFFER_DESIRED_SIZE  // default size for Buffer
#define BUFFER_GROWTH_FACTOR                      2  // how Buffer grows when needed

#define BUFFER_FLAG_SET(b, f) do { (b)->flg |= ( f); } while (0)
#define BUFFER_FLAG_CLR(b, f) do { (b)->flg &= (~f); } while (0)
#define BUFFER_FLAG_CHK(b, f)    ( (b)->flg &  ( f) )

#define buffer_ensure_extra(b, extra) \
    do { \
        Size total = (extra) + (b)->pos; \
        if (total > (b)->cap) { \
            buffer_ensure_total(b, total); \
        } \
    } while (0)

static void buffer_ensure_total(Buffer* b, Size total);
static void buffer_realloc(Buffer* b, Size cap);
static void buffer_dump_file(Buffer* b, FILE* fp);

// TODO: might want to specify a capacity to pre-allocate it
Buffer* buffer_build(void) {
    Buffer* b = (Buffer*) malloc(sizeof(Buffer));
    buffer_init(b);
    BUFFER_FLAG_SET(b, BUFFER_FLAG_BUF_IN_HEAP);
    return b;
}

void buffer_init(Buffer* b) {
    b->ptr = b->buf;
    b->cap = BUFFER_DATA_SIZE;
    b->pos = 0;
    b->flg = 0;
}

void buffer_destroy(Buffer* b) {
    if (BUFFER_FLAG_CHK(b, BUFFER_FLAG_PTR_IN_HEAP)) {
        BUFFER_FLAG_CLR(b, BUFFER_FLAG_PTR_IN_HEAP);
        LOG_DEBUG("DESTROY deleting heap-allocated ptr with %u bytes", b->cap);
        free(b->ptr);
    }
    if (BUFFER_FLAG_CHK(b, BUFFER_FLAG_BUF_IN_HEAP)) {
        BUFFER_FLAG_CLR(b, BUFFER_FLAG_BUF_IN_HEAP);
        LOG_DEBUG("DESTROY deleting heap-allocated buf");
        free(b);
    }
}

Buffer* buffer_clone(const Buffer* b) {
    Slice s = buffer_get_slice(b);
    Buffer* n = buffer_build();
    buffer_append_slice(n, s);
    return n;
}

void buffer_dump(Buffer* b) {
    buffer_dump_file(b, stderr);
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

void buffer_pack(Buffer* b) {
    if (!BUFFER_FLAG_CHK(b, BUFFER_FLAG_PTR_IN_HEAP)) {
        // Buffer is using stack-allocated buf, don't touch
        return;
    }

    // Buffer is using heap-allocated ptr
    if (b->pos < BUFFER_DATA_SIZE) {
        // Enough space in buf, switch to it
        LOG_DEBUG("MIGRATE heap [%p] to stack [%p]", b->ptr, b->buf);
        memcpy(b->buf, b->ptr, b->pos);
        buffer_realloc(b, 0);
        b->ptr = b->buf;
        BUFFER_FLAG_CLR(b, BUFFER_FLAG_PTR_IN_HEAP);
    } else {
        // Not enough space in buf, remain in ptr
        buffer_realloc(b, b->pos);
    }
}

void buffer_clear(Buffer* b) {
    b->pos = 0;
}

void buffer_append_byte(Buffer* b, Byte u) {
    buffer_ensure_extra(b, 1);
    LOG_DEBUG("APPENDB [0x%02x:%c]", (Size) u, isprint(u) ? u : '.');
    b->ptr[b->pos++] = u;
}

void buffer_append_slice(Buffer* b, Slice s) {
    buffer_ensure_extra(b, s.len);
    LOG_DEBUG("APPENDS [%u:%.*s]", s.len, s.len, s.ptr);
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

/*
 * potentially expensive because it calls sprintf TWICE:
 * 1. to determine space required to format all args and ensure there is enough space
 * 2. to actually generate the results
 */
void buffer_format_print(Buffer* b, const char* fmt, ...) {
    va_list ap1;
    va_list ap2;

    va_start(ap1, fmt);
    va_copy(ap2, ap1);

    Size size = vsnprintf(0, 0, fmt, ap1);
    va_end(ap1);
    LOG_DEBUG("FORMAT [%s] => requires %d bytes", fmt, size);

    buffer_ensure_extra(b, size + 1);  // vsnprintf below will also include a '\0'

    vsnprintf((char*) b->ptr + b->pos, size + 1, fmt, ap2);
    b->pos += size;
    va_end(ap2);
    LOG_DEBUG("FORMAT [%d:%.*s]", size, size, b->ptr + b->pos);
}


static void buffer_ensure_total(Buffer* b, Size total) {
    Size changes = 0;
    Size current = b->cap;
    LOG_DEBUG("ENSURE currently using %u out of %u", b->pos, b->cap);
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
            // Buffer is using stack-allocated buf and need more, switch to ptr
            Size old = b->cap;
            b->ptr = 0;
            b->cap = 0;
            buffer_realloc(b, current);
            memcpy(b->ptr, b->buf, old);
            LOG_DEBUG("MIGRATE stack [%u:%p] to heap [%u:%p]", old, b->buf, current, b->ptr);
            BUFFER_FLAG_SET(b, BUFFER_FLAG_PTR_IN_HEAP);
        }
    }
    assert(b->cap >= total);
}

static void buffer_realloc(Buffer* b, Size cap) {
    LOG_DEBUG("REALLOC %p: %u to %u bytes", b->ptr, b->cap, cap);
    Byte* tmp = realloc((void*) b->ptr, cap);
    if (!tmp) {
        LOG_ERROR("Out of memory when reallocating %p from %u to %u bytes", b->ptr, b->cap, cap);
    }
    b->ptr = tmp;
    b->cap = cap;
    return;
}

static void buffer_dump_file(Buffer* b, FILE* fp) {
    fprintf(fp, "buffer:");
    fprintf(fp, " %s %c,", "BUF_IN_HEAP", BUFFER_FLAG_CHK(b, BUFFER_FLAG_BUF_IN_HEAP) ? 'Y' : 'N');
    fprintf(fp, " %s %c,", "PTR_IN_HEAP", BUFFER_FLAG_CHK(b, BUFFER_FLAG_PTR_IN_HEAP) ? 'Y' : 'N');
    fprintf(fp, " buf %p, ptr %p, cap %u, pos %u\n", b->buf, b->ptr, b->cap, b->pos);
    Slice s = buffer_get_slice(b);
    slice_dump(s);
}
