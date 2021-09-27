#ifndef UTIL_H_
#define UTIL_H_

/*
 * Utility functions.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

// We just use a pointer
struct Buffer;

// Dump a block of bytes into a FILE stream with nice formatting.
void dump_bytes(FILE* fp, const void* ptr, size_t len);

// Format a number of microseconds (us) into a Buffer, in detailed or
// abbreviated form.
void format_detailed_us(unsigned long us, struct Buffer* b);
void format_abbreviated_us(unsigned long us, struct Buffer* b);

// Realloc a chunk of memory to a given size.  Useful to:
// len == 0            free memory pointed to by ptr
// len > 0, ptr == 0   allocate a new chunk of memory of len bytes
// len > 0, ptr != 0   reallocate a chunk of memory of len bytes
void* memory_realloc(void* ptr, size_t len);

#endif
