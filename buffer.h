#ifndef BUFFER_H_
#define BUFFER_H_

/*
 * Buffer -- a purely reference type
 * write-only access to an array of bytes.
 * Space is automatically grown as needed -- hopefully no overflows.
 * Small buffers use an internal array, larger buffers allocate.
 * User always looks at buffer->ptr, whether small or large.
 * It does NOT add a null terminator at the end.
 * Do NOT use with C standard strXXX() functions.
 *
 * TODO
 *
 * trim -- for a set of characters?
 * tolower, toupper
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include "slice.h"

#define BUFFER_FLAG_SET(b, f) do { (b)->flg |= ( f); } while (0)
#define BUFFER_FLAG_CLR(b, f) do { (b)->flg &= (~f); } while (0)
#define BUFFER_FLAG_CHK(b, f)    ( (b)->flg &  ( f) )

// Flags used for a Buffer.
#define BUFFER_FLAG_PTR_IN_HEAP (1U<<0)

// Total size we want Buffer struct to have.
#define BUFFER_DESIRED_SIZE 64UL

// Total size used up by Buffer fields, EXCEPT buf.
// MUST BE KEPT IN SYNC WITH DECLARATION OF struct Buffer.
#define BUFFER_FIELDS_SIZE (\
    sizeof(uint8_t*)  /* ptr */ + \
    sizeof(uint32_t)  /* cap */ + \
    sizeof(uint32_t)  /* len */ + \
    sizeof(uint8_t)   /* flg */ + \
    0)

// Size allowed for a Buffer's static data array.
#define BUFFER_DATA_SIZE (BUFFER_DESIRED_SIZE - BUFFER_FIELDS_SIZE)

typedef struct Buffer {
    uint8_t* ptr;                    // pointer to beginning of data
    uint32_t cap;                    // total data capacity
    uint32_t len;                    // current buffer length
    uint8_t flg;                     // flags for Buffer
    uint8_t buf[BUFFER_DATA_SIZE];   // stack space for small Buffer
} Buffer;

#if __STDC_VERSION__ >= 201112L
// In C11 we can make sure Buffer ended up with desired size:
#include <assert.h>
static_assert(sizeof(Buffer) == BUFFER_DESIRED_SIZE, "Buffer has wrong size");
#endif

// Clear the contents of Buffer -- does NOT reallocate current memory.
#define buffer_clear(b) \
    do { \
        (b)->len = 0; \
    } while (0)

// Allocate a Buffer on heap memory.
// After allocation, run the constructor on the buffer.
Buffer* buffer_allocate(size_t cap);

// Release a Buffer previously allocated on the heap.
// Before releasing, run the destructor on the buffer.
void buffer_release(Buffer* b);

// Buffer constructor.
// Can be run on a Buffer from the stack or heap.
// Optionally pass in a minimum capacity; 0 means use the default.
void buffer_build(Buffer* b, size_t cap);

// Buffer destructor.
// Can be run on a Buffer from the stack or heap.
void buffer_destroy(Buffer* b);

// Clone an existing Buffer into another.
void buffer_clone(const Buffer* b, Buffer* t);

// Ensure buffer has space for total bytes.
void buffer_ensure_total(Buffer* b, uint32_t total);

// Reallocate memory so that current data fits exactly into Buffer.
void buffer_pack(Buffer* b);

// Append a single byte to current contents of Buffer.
void buffer_append_byte(Buffer* b, uint8_t u);

// Append a string of given length to current contents of Buffer.
// If len < 0, use null terminator, otherwise copy len bytes.
void buffer_append_string(Buffer* b, const char* str, int len);

// Append a slice to current contents of Buffer.
void buffer_append_slice(Buffer* b, Slice s);

// Append a buffer to current contents of Buffer.
void buffer_append_buffer(Buffer* b, const Buffer* buf);

// Append a formatted signed / unsigned integer to current contents of Buffer.
void buffer_format_signed(Buffer* b, long long l);
void buffer_format_unsigned(Buffer* b, unsigned long long l);

// Append a formatted double to current contents of Buffer.
void buffer_format_double(Buffer* b, double d);

// Append a printf-formatted string to current contents of Buffer.
// THESE ARE EXPENSIVE.
void buffer_format_vprint(Buffer* b, const char* fmt, va_list ap);
void buffer_format_print(Buffer* b, const char* fmt, ...);

#endif
