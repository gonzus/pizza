#ifndef BUFFER_H_
#define BUFFER_H_

/*
 * TODO
 *
 * MAYBE?
 *
 * trim -- for a set of characters?
 * tolower
 * toupper
 * sprintf
 * memory allocation functions?
 */

#include <slice.h>

/*
 * buffer -- write-only access to an array of bytes
 * it does NOT add a null terminator at the end
 * do NOT use with C standard strXXX() functions
 */
typedef struct buffer {
    Byte* ptr;          // pointer to beginning of data
    unsigned int cap;   // total data capacity
    unsigned int pos;   // position of next char written to buffer
} buffer;

// build an empty / default-sized buffer
buffer* buffer_build(void);

// destroy a buffer
void buffer_destroy(buffer* b);

// get current buffer length
unsigned int buffer_get_length(const buffer* b);

// get current buffer capacity
unsigned int buffer_get_capacity(const buffer* b);

// get a slice to current contents of buffer
slice buffer_get_slice(const buffer* b);

// clone an existing buffer
buffer* buffer_clone(const buffer* b);

// reallocate memory so that current data fits exactly into buffer
void buffer_pack(buffer* b);

// clear the contents of buffer -- does NOT reallocate current memory
void buffer_clear(buffer* b);

// append to current contents of buffer a single byte
void buffer_append_byte(buffer* b, Byte u);

// append to current contents of buffer a slice
void buffer_append_slice(buffer* b, slice s);

// append to current contents of buffer a formatted signed / unsigned integer
void buffer_format_signed(buffer* b, long long l);
void buffer_format_unsigned(buffer* b, unsigned long long l);

// append to current contents of buffer a formatted double
void buffer_format_double(buffer* b, double d);

#endif
