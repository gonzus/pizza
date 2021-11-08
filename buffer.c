#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "buffer.h"

#if !defined(va_copy)
// need this if we want to compile for C89
#define va_copy(d,s) __va_copy(d,s)
#endif

#define BUFFER_DEFAULT_CAPACITY BUFFER_DESIRED_SIZE  // default size for Buffer
#define BUFFER_GROWTH_FACTOR                      2  // how Buffer grows when needed

static void buffer_adjust(Buffer* b, uint32_t cap);
static void buffer_append_ptr_len(Buffer* b, const char* ptr, uint32_t len);

void buffer_build(Buffer* b) {
    memset(b, 0, sizeof(Buffer));
    b->ptr = b->buf;
    b->cap = BUFFER_DATA_SIZE;
}

void buffer_destroy(Buffer* b) {
    if (BUFFER_FLAG_CHK(b, BUFFER_FLAG_PTR_IN_HEAP)) {
        BUFFER_FLAG_CLR(b, BUFFER_FLAG_PTR_IN_HEAP);
        MEMORY_FREE_ARRAY(b->ptr, char, b->cap);
        assert(!b->ptr);
    }
}

Buffer* buffer_allocate(void) {
    Buffer* b = 0; MEMORY_ALLOC(b, Buffer);
    buffer_build(b);
    BUFFER_FLAG_SET(b, BUFFER_FLAG_BUF_IN_HEAP);
    assert(b);
    return b;
}

void buffer_release(Buffer* b) {
    assert(BUFFER_FLAG_CHK(b, BUFFER_FLAG_BUF_IN_HEAP));
    BUFFER_FLAG_CLR(b, BUFFER_FLAG_BUF_IN_HEAP);
    buffer_destroy(b);
    MEMORY_FREE(b, Buffer);
    assert(!b);
}

void buffer_ensure_total(Buffer* b, uint32_t total) {
    uint32_t changes = 0;
    uint32_t current = b->cap;
    while (total > current) {
        ++changes;
        uint32_t next = current == 0 ? BUFFER_DEFAULT_CAPACITY : current * BUFFER_GROWTH_FACTOR;
        current = next;
    }
    if (changes) {
        if (BUFFER_FLAG_CHK(b, BUFFER_FLAG_PTR_IN_HEAP)) {
            buffer_adjust(b, current);
        } else {
            // Buffer is using stack-allocated buf and need more, switch to ptr
            uint32_t old = b->cap;
            b->ptr = 0;
            b->cap = 0;
            buffer_adjust(b, current);
            memcpy(b->ptr, b->buf, old);
            BUFFER_FLAG_SET(b, BUFFER_FLAG_PTR_IN_HEAP);
        }
    }
}

Buffer* buffer_clone(const Buffer* b) {
    Buffer* n = buffer_allocate();
    buffer_append_buffer(n, b);
    return n;
}

Slice buffer_slice(const Buffer* b) {
    Slice s = slice_from_memory(b->ptr, b->len);
    return s;
}

void buffer_pack(Buffer* b) {
    if (!BUFFER_FLAG_CHK(b, BUFFER_FLAG_PTR_IN_HEAP)) {
        // Buffer is using stack-allocated buf, don't touch
        return;
    }

    // Buffer is using heap-allocated ptr
    if (b->len < BUFFER_DATA_SIZE) {
        // Enough space in buf, switch to it
        memcpy(b->buf, b->ptr, b->len);
        buffer_adjust(b, 0);
        b->ptr = b->buf;
        BUFFER_FLAG_CLR(b, BUFFER_FLAG_PTR_IN_HEAP);
    } else {
        // Not enough space in buf, remain in ptr
        buffer_adjust(b, b->len);
    }
}

void buffer_set_to_slice(Buffer* b, Slice s, bool zero) {
    buffer_clear(b);
    buffer_append_slice(b, s);
    if (zero) {
        buffer_null_terminate(b);
    }
}

void buffer_append_byte(Buffer* b, char t) {
    buffer_ensure_extra(b, 1);
    b->ptr[b->len++] = t;
}

void buffer_append_string(Buffer* b, const char* str, int len) {
    if (len <= 0) {
        len = str ? strlen(str) : 0;
    }
    buffer_append_ptr_len(b, str, len);
}

void buffer_append_slice(Buffer* b, Slice s) {
    buffer_append_ptr_len(b, s.ptr, s.len);
}

void buffer_append_buffer(Buffer* b, const Buffer* buf) {
    buffer_append_ptr_len(b, buf->ptr, buf->len);
}

void buffer_format_signed(Buffer* b, long long l) {
    char cstr[99];
    uint32_t clen = sprintf(cstr, "%lld", l);
    buffer_append_ptr_len(b, cstr, clen);
}

void buffer_format_unsigned(Buffer* b, unsigned long long l) {
    char cstr[99];
    uint32_t clen = sprintf(cstr, "%llu", l);
    buffer_append_ptr_len(b, cstr, clen);
}

void buffer_format_double(Buffer* b, double d) {
    char cstr[99];
    uint32_t clen = sprintf(cstr, "%f", d);
    buffer_append_ptr_len(b, cstr, clen);
}

/*
 * potentially expensive because it calls sprintf TWICE:
 * 1. to determine space required to format all args and ensure there is enough space
 * 2. to actually generate the results
 */
void buffer_format_vprint(Buffer* b, const char* fmt, va_list ap) {
    va_list aq;
    va_copy(aq, ap);

    uint32_t size = vsnprintf(0, 0, fmt, ap);
    va_end(ap);

    buffer_ensure_extra(b, size + 1);  // vsnprintf below will also include a '\0'

    vsnprintf((char*) b->ptr + b->len, size + 1, fmt, aq);
    b->len += size;
    va_end(aq);
}

void buffer_format_print(Buffer* b, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    buffer_format_vprint(b, fmt, ap);
    va_end(ap);
}

static void buffer_adjust(Buffer* b, uint32_t cap) {
    char* tmp = 0;
    MEMORY_ADJUST(tmp, b->ptr, b->cap, cap);
    b->ptr = tmp;
    b->cap = cap;
}

static void buffer_append_ptr_len(Buffer* b, const char* ptr, uint32_t len) {
    if (len <= 0) {
        return;
    }
    buffer_ensure_extra(b, len);
    memcpy(b->ptr + b->len, ptr, len);
    b->len += len;
}
