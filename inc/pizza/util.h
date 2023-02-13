#ifndef UTIL_H_
#define UTIL_H_

/*
 * Utility functions.
 */

// Mark an arg / variable as unused.
#define UNUSED(x) (void) x

// We just use a pointer
struct Buffer;

// Format a number of microseconds (us) into a Buffer, in detailed or
// abbreviated form.
void format_detailed_us(unsigned long us, struct Buffer* b);
void format_abbreviated_us(unsigned long us, struct Buffer* b);

#endif
