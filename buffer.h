#ifndef BUFFER_H_
#define BUFFER_H_

#include <slice.h>

/*
 * =======================================================================
 * buffer -- write-only access to an array of bytes
 * it does NOT add a null terminator at the end
 * do NOT use with C standard strXXX() functions
 */
typedef struct buffer {
    Byte* ptr;          // pointer to beginning of data
    unsigned int cap;   // total capacity of data
    unsigned int pos;   // position of next char written to data
} buffer;

// build an empty / default-sized buffer
buffer* buffer_build(void);

// destroy a buffer
void buffer_destroy(buffer* b);

// get a slice to current contents of buffer
slice buffer_get_slice(const buffer* b);

// get current buffer length
unsigned int buffer_get_length(const buffer* b);

// get current buffer capacity
unsigned int buffer_get_capacity(const buffer* b);

// clear the contents of buffer -- does NOT reallocate current memory
void buffer_clear(buffer* b);

// discard current contents of buffer and set them to a single byte
buffer* buffer_set_byte(buffer* b, Byte u);

// discard current contents of buffer and set them to a slice
buffer* buffer_set_slice(buffer* b, slice s);

// append to current contents of buffer a single byte
buffer* buffer_append_byte(buffer* b, Byte u);

// append to current contents of buffer a slice
buffer* buffer_append_slice(buffer* b, slice s);

#endif
