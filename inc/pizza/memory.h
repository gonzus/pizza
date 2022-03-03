#ifndef MEMORY_H_
#define MEMORY_H_

/*
 * Functions to deal with memory.
 */

#include <stddef.h>
#include <string.h>

#define MEMORY_ZERO_OUT 1

#if MEMORY_ZERO_OUT
#define MEMORY_CLEAR(ptr, len) memset(ptr, 0, len)
#else
#define MEMORY_CLEAR(ptr, len) (void) (ptr)
#endif

#define MEMORY_ALLOC(var, type) MEMORY_ALLOC_ARRAY(var, type, 1)

#define MEMORY_ALLOC_ARRAY(var, type, count) \
    do { \
        var = (type*) memory_realloc(0, count * sizeof(type)); \
        MEMORY_CLEAR(var, count * sizeof(type)); \
    } while (0)

#define MEMORY_FREE(var, type) MEMORY_FREE_ARRAY(var, type, 1)

#define MEMORY_FREE_ARRAY(var, type, count) \
    do { \
        MEMORY_CLEAR(var, count * sizeof(type)); \
        var = memory_realloc(var, 0); \
    } while (0)

#define MEMORY_ADJUST(var, ptr, olen, nlen) \
    do { \
        if (nlen < olen) { \
            MEMORY_CLEAR(ptr + nlen, olen - nlen); \
        } \
        var = memory_realloc(ptr, nlen); \
        if (nlen > olen) { \
            MEMORY_CLEAR(var + olen, nlen - olen); \
        } \
    } while (0)

// Dump a block of bytes into stderr with nice formatting.
void dump_bytes(const void* ptr, size_t len);

// Realloc a chunk of memory to a given size.  Useful to:
// len == 0            free memory pointed to by ptr
// len > 0, ptr == 0   allocate a new chunk of memory of len bytes
// len > 0, ptr != 0   reallocate memory of len bytes (larger or smaller)
void* memory_realloc(void* ptr, size_t len);

#endif
