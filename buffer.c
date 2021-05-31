#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"

#if !defined(va_copy)
// need this if we want to compile for C89
#define va_copy(d,s) __va_copy(d,s)
#endif

#define BUFFER_DEFAULT_CAPACITY BUFFER_DESIRED_SIZE  // default size for Buffer
#define BUFFER_GROWTH_FACTOR                      2  // how Buffer grows when needed

#define buffer_ensure_extra(b, extra) \
    do { \
        uint32_t total = (extra) + (b)->len; \
        if (total > (b)->cap) { \
            buffer_ensure_total(b, total); \
        } \
    } while (0)

static void buffer_ensure_total(Buffer* b, uint32_t total);
static void buffer_adjust(Buffer* b, uint32_t cap);
static void* buffer_realloc(void* ptr, uint32_t len);

Buffer* buffer_build(void) {
    Buffer* b = buffer_realloc(0, sizeof(Buffer));
    buffer_init(b);
    BUFFER_FLAG_SET(b, BUFFER_FLAG_BUF_IN_HEAP);
    return b;
}

Buffer* buffer_build_capacity(uint32_t cap) {
    Buffer* b = buffer_build();
    buffer_ensure_total(b, cap);
    return b;
}

void buffer_init(Buffer* b) {
    b->ptr = b->buf;
    b->cap = BUFFER_DATA_SIZE;
    b->len = 0;
    b->flg = 0;
}

void buffer_destroy(Buffer* b) {
    if (BUFFER_FLAG_CHK(b, BUFFER_FLAG_PTR_IN_HEAP)) {
        BUFFER_FLAG_CLR(b, BUFFER_FLAG_PTR_IN_HEAP);
        b->ptr = buffer_realloc(b->ptr, 0);
        assert(!b->ptr);
    }
    if (BUFFER_FLAG_CHK(b, BUFFER_FLAG_BUF_IN_HEAP)) {
        BUFFER_FLAG_CLR(b, BUFFER_FLAG_BUF_IN_HEAP);
        b = buffer_realloc(b, 0);
        assert(!b);
    }
}

Buffer* buffer_clone(const Buffer* b) {
    Buffer* n = buffer_build();
    buffer_append_string(n, (const char*) b->ptr, b->len);
    return n;
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

void buffer_append_byte(Buffer* b, uint8_t u) {
    buffer_ensure_extra(b, 1);
    b->ptr[b->len++] = u;
}

void buffer_append_string(Buffer* b, const char* str, int len) {
    if (len < 0) {
        len = str ? strlen(str) : 0;
    }
    if (len <= 0) {
        return;
    }
    buffer_ensure_extra(b, len);
    memcpy(b->ptr + b->len, str, len);
    b->len += len;
}

void buffer_format_signed(Buffer* b, long long l) {
    char cstr[99];
    uint32_t clen = sprintf(cstr, "%lld", l);
    buffer_append_string(b, cstr, clen);
}

void buffer_format_unsigned(Buffer* b, unsigned long long l) {
    char cstr[99];
    uint32_t clen = sprintf(cstr, "%llu", l);
    buffer_append_string(b, cstr, clen);
}

void buffer_format_double(Buffer* b, double d) {
    char cstr[99];
    uint32_t clen = sprintf(cstr, "%f", d);
    buffer_append_string(b, cstr, clen);
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


static void buffer_ensure_total(Buffer* b, uint32_t total) {
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
    assert(b->cap >= total);
}

static void buffer_adjust(Buffer* b, uint32_t cap) {
    uint8_t* tmp = buffer_realloc(b->ptr, cap);
    b->ptr = tmp;
    b->cap = cap;
}

static void* buffer_realloc(void* ptr, uint32_t len) {
    // It seems realloc() works differently between MacOs and Linux.
    // Since the intention when passing len==0 is to free the memory,
    // lets just do it and return NULL, which is the expected behaviour.
    if (len == 0) {
        free(ptr);
        return NULL;
    } else {
        void* tmp = (void*) realloc(ptr, len);
        if ((tmp && len > 0) || (!tmp && len <= 0)) {
            return tmp;  // all good
        }
    }

    if (errno) {
        fprintf(stderr, "Error %d (%s) calling realloc(%p, %u)",
                errno, strerror(errno), ptr, len);
    } else {
        fprintf(stderr, "Could not allocate memory calling realloc(%p, %u)\n",
                ptr, len);
    }
    abort();
    return 0;
}
