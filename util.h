#ifndef UTIL_H_
#define UTIL_H_

/*
 * Utility functions.
 */

// We just use a pointer
struct Buffer;

#include <stdio.h>

// dump a range of bytes into fp, in a readable way
void dump_bytes(FILE* fp, const unsigned char* bdat, int blen);

// format a number of microseconds into a Buffer in a detailed manner
void format_detailed_us(unsigned long us, struct Buffer* b);

// format a number of microseconds into a Buffer in an abbreviated manner
void format_abbreviated_us(unsigned long us, struct Buffer* b);

#endif
