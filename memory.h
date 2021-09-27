#ifndef MEMORY_H_
#define MEMORY_H_

/*
 * Functions to deal with memory.
 */

#include <stdio.h>
#include <stddef.h>

// Dump a block of bytes into a FILE stream with nice formatting.
void dump_bytes(FILE* fp, const void* ptr, size_t len);

// Realloc a chunk of memory to a given size.  Useful to:
// len == 0            free memory pointed to by ptr
// len > 0, ptr == 0   allocate a new chunk of memory of len bytes
// len > 0, ptr != 0   reallocate a chunk of memory of len bytes
void* memory_realloc(void* ptr, size_t len);

#endif
