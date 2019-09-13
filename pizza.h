#ifndef PIZZA_H_
#define PIZZA_H_

/*
 * =======================================================================
 * slices -- read-only access to a char*
 * it does NOT assume data has a null terminator at the end
 * do NOT use with C standard strXXX() functions
 * intended to be passed around by copying, not with a reference (pointer)
 */
typedef struct slice {
    const char* ptr;    // pointer to beginning of data
    unsigned int len;   // length of data
} slice;

// wrap a const char* into a slice; compute the length using strlen()
slice slice_wrap_ptr(const char* p);

// wrap a given number of bytes from a const char* into a slice.
slice slice_wrap_ptr_len(const char* p, unsigned int l);

/*
 * copy a slice into a string buffer and return the buffer; includes a null terminator
 * the string buffer must already have the right size (slice.len + 1)
 */
const char* slice_to_string(slice s, char* string);

// dump a slice into stderr
void slice_dump(slice s);


/*
 * =======================================================================
 * buffer -- write-only access to a char*
 * it does NOT add a null terminator at the end
 * do NOT use with C standard strXXX() functions
 */
typedef struct buffer {
    char* ptr;          // pointer to beginning of data
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

// discard current contents of buffer and set them to a single character
buffer* buffer_set_character(buffer* b, char c);

// discard current contents of buffer and set them to a slice
buffer* buffer_set_slice(buffer* b, slice s);

// append to current contents of buffer a single character
buffer* buffer_append_character(buffer* b, char c);

// append to current contents of buffer a slice
buffer* buffer_append_slice(buffer* b, slice s);

#endif
