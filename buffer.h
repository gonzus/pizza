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
 * append_XXX for int, long, float, etc
 * sprintf
 */

#include <slice.h>

/*
 * =======================================================================
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

// clone an existing buffer
buffer* buffer_clone(const buffer* b);

// destroy a buffer
void buffer_destroy(buffer* b);

// reallocate memory so that current data fits exactly into buffer
unsigned int buffer_pack(buffer* b);

// get a slice to current contents of buffer
slice buffer_get_slice(const buffer* b);

// get current buffer length
unsigned int buffer_get_length(const buffer* b);

// get current buffer capacity
unsigned int buffer_get_capacity(const buffer* b);

// clear the contents of buffer -- does NOT reallocate current memory
void buffer_clear(buffer* b);

// append to current contents of buffer a single byte
buffer* buffer_append_byte(buffer* b, Byte u);

// append to current contents of buffer a slice
buffer* buffer_append_slice(buffer* b, slice s);

#endif
