#ifndef UTIL_H_
#define UTIL_H_

/*
 * Utility functions.
 */

// We just use a pointer
struct Buffer;

#include <stdio.h>

void dump_bytes(FILE* fp, const unsigned char* bdat, int blen);

void format_detailed_us(unsigned long us, struct Buffer* b);
void format_abbreviated_us(unsigned long us, struct Buffer* b);

#endif
