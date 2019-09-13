#ifndef PIZZA_H_
#define PIZZA_H_

typedef unsigned char Byte;

/*
 * =======================================================================
 * slices -- read-only access to an array of bytes
 * it does NOT assume data has a null terminator at the end
 * do NOT use with C standard strXXX() functions
 * intended to be passed around by copying, not with a reference (pointer)
 */
typedef struct slice {
    const Byte* ptr;    // pointer to beginning of data
    unsigned int len;   // length of data
} slice;

// the null slice
extern slice SLICE_NULL;

// return true if slice is null (no ptr)
int slice_is_null(slice s);

// return true if slice is empty (valid ptr, zero len)
int slice_is_empty(slice s);

// return slice length
int slice_get_length(slice s);

// wrap a string (const char*) into a slice; compute the length using strlen()
slice slice_wrap_string(const char* string);

// wrap a given number of bytes from a string (const char*) into a slice.
slice slice_wrap_string_length(const char* string, unsigned int length);

/*
 * copy a slice into a string (char*) and return the buffer; includes a null terminator
 * the string buffer must already have the right size (slice.len + 1)
 */
unsigned int slice_to_string(slice s, char* string);

int slice_tokenize(slice s, slice sep, slice* c);

// dump a slice into stderr
void slice_dump(slice s);


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
