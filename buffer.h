#ifndef BUFFER_H_
#define BUFFER_H_

/*
 * Buffer -- write-only access to an array of bytes.
 * Space is automatically grown as needed -- hopefully no overflows.
 * Small buffers use an internal array, larger buffers allocate.
 * It does NOT add a null terminator at the end.
 * Do NOT use with C standard strXXX() functions.
 *
 * TODO
 *
 * trim -- for a set of characters?
 * tolower, toupper
 * memory allocation functions?
 */

#include <slice.h>

// Flags used for a Buffer.
#define BUFFER_FLAG_BUF_IN_HEAP (1U<<0)
#define BUFFER_FLAG_PTR_IN_HEAP (1U<<1)

// Total size we want Buffer struct to have.
#define BUFFER_DESIRED_SIZE 64UL

// Total size used up by Buffer fields, EXCEPT buf.
// MUST BE KEPT IN SYNC WITH DECLARATION OF struct Buffer.
#define BUFFER_FIELDS_SIZE (\
    sizeof(Byte*)  /* ptr */ + \
    sizeof(Size)   /* cap */ + \
    sizeof(Size)   /* pos */ + \
    sizeof(Byte)   /* flg */ + \
    0)

// Size allowed for a Buffer's static data array.
#define BUFFER_DATA_SIZE (BUFFER_DESIRED_SIZE - BUFFER_FIELDS_SIZE)

typedef struct Buffer {
    Byte* ptr;                    // pointer to beginning of data
    Size cap;                     // total data capacity
    Size pos;                     // position of next char written to Buffer
    Byte flg;                     // flags for Buffer
    Byte buf[BUFFER_DATA_SIZE];   // stack space for small Buffer
} Buffer;

#if __STDC_VERSION__ >= 201112L
// In C11 we can make sure Buffer ended up with desired size:
#include <assert.h>
static_assert(sizeof(Buffer) == BUFFER_DESIRED_SIZE, "Buffer has wrong size");
#endif

// Build an empty / default-sized Buffer.
Buffer* buffer_build(void);

// Initialize Buffer, whether allocated in stack or heap.
void buffer_init(Buffer* b);

// Destroy a Buffer.
void buffer_destroy(Buffer* b);

// Clone an existing Buffer.
Buffer* buffer_clone(const Buffer* b);

// Dump a Buffer to stderr.
void buffer_dump(Buffer* s);

// Get current Buffer length.
Size buffer_length(const Buffer* b);

// Get current Buffer capacity.
Size buffer_capacity(const Buffer* b);

// Get a Slice to current contents of Buffer.
Slice buffer_get_slice(const Buffer* b);

// Clear the contents of Buffer -- does NOT reallocate current memory.
void buffer_clear(Buffer* b);

// Reallocate memory so that current data fits exactly into Buffer.
void buffer_pack(Buffer* b);

// Append a single byte to current contents of Buffer.
void buffer_append_byte(Buffer* b, Byte u);

// Append a Slice to current contents of Buffer.
void buffer_append_slice(Buffer* b, Slice s);

// Append a formatted signed / unsigned integer to current contents of Buffer.
void buffer_format_signed(Buffer* b, long long l);
void buffer_format_unsigned(Buffer* b, unsigned long long l);

// Append a formatted double to current contents of Buffer.
void buffer_format_double(Buffer* b, double d);

// Append a printf-formatted string to current contents of Buffer.
void buffer_format_print(Buffer* b, const char* fmt, ...);

#endif
