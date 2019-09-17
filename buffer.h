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
 * memory allocation functions?
 */

#include <slice.h>

/*
 * Buffer -- write-only access to an array of bytes
 * it does NOT add a null terminator at the end
 * do NOT use with C standard strXXX() functions
 */

// total size we want Buffer struct to have
#define BUFFER_DESIRED_SIZE 64UL

// total size used up by Buffer fields, except buf
// MUST BE KEPT IN SYNC WITH DECLARATION OF struct Buffer
#define BUFFER_FIELDS_SIZE (\
    sizeof(Byte*) + \
    sizeof(Size)  + \
    sizeof(Size)  + \
    sizeof(Byte)  + \
    0)

// size allowed for a Buffer's static data array
#define BUFFER_DATA_SIZE (BUFFER_DESIRED_SIZE - BUFFER_FIELDS_SIZE)

typedef struct Buffer {
    Byte* ptr;                    // pointer to beginning of data
    Size cap;                     // total data capacity
    Size pos;                     // position of next char written to Buffer
    Byte flg;                     // flags for Buffer
    Byte buf[BUFFER_DATA_SIZE];   // stack space for small Buffer
} Buffer;

#if __STDC_VERSION__ >= 201112L
#include <assert.h>
static_assert(sizeof(Buffer) == BUFFER_DESIRED_SIZE, "Buffer has wrong size");
#endif

// build an empty / default-sized Buffer
Buffer* buffer_build(void);

// destroy a Buffer
void buffer_destroy(Buffer* b);

// get current Buffer length
Size buffer_get_length(const Buffer* b);

// get current Buffer capacity
Size buffer_get_capacity(const Buffer* b);

// get a Slice to current contents of Buffer
Slice buffer_get_slice(const Buffer* b);

// clone an existing Buffer
Buffer* buffer_clone(const Buffer* b);

// reallocate memory so that current data fits exactly into Buffer
void buffer_pack(Buffer* b);

// clear the contents of Buffer -- does NOT reallocate current memory
void buffer_clear(Buffer* b);

// append to current contents of Buffer a single byte
void buffer_append_byte(Buffer* b, Byte u);

// append to current contents of Buffer a Slice
void buffer_append_slice(Buffer* b, Slice s);

// append to current contents of Buffer a formatted signed / unsigned integer
void buffer_format_signed(Buffer* b, long long l);
void buffer_format_unsigned(Buffer* b, unsigned long long l);

// append to current contents of Buffer a formatted double
void buffer_format_double(Buffer* b, double d);

// append to current contents of Buffer using a printf-like format
void buffer_format(Buffer* b, const char* fmt, ...);

#endif
