#ifndef BUFFER_H_
#define BUFFER_H_

/*
 * Buffer -- write-only access to an array of bytes.
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

#ifndef PIZZA_BYTE_SIZE
#define PIZZA_BYTE_SIZE
#include <stdint.h>
typedef uint8_t  Byte;  // type for bytes (0..255 = 2^8-1)
typedef uint32_t Size;  // type for sizes (0..2^32-1)
#endif

#define BUFFER_FLAG_SET(b, f) do { (b)->flg |= ( f); } while (0)
#define BUFFER_FLAG_CLR(b, f) do { (b)->flg &= (~f); } while (0)
#define BUFFER_FLAG_CHK(b, f)    ( (b)->flg &  ( f) )

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
    sizeof(Size)   /* len */ + \
    sizeof(Byte)   /* flg */ + \
    0)

// Size allowed for a Buffer's static data array.
#define BUFFER_DATA_SIZE (BUFFER_DESIRED_SIZE - BUFFER_FIELDS_SIZE)

typedef struct Buffer {
    Byte* ptr;                    // pointer to beginning of data
    Size cap;                     // total data capacity
    Size len;                     // current buffer length
    Byte flg;                     // flags for Buffer
    Byte buf[BUFFER_DATA_SIZE];   // stack space for small Buffer
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

// Build an empty / default-sized Buffer.
Buffer* buffer_build(void);

// Build a Buffer with defined capacity.
Buffer* buffer_build_capacity(Size cap);

// Initialize Buffer, whether allocated in stack or heap.
void buffer_init(Buffer* b);

// Destroy a Buffer.
void buffer_destroy(Buffer* b);

// Clone an existing Buffer.
Buffer* buffer_clone(const Buffer* b);

// Reallocate memory so that current data fits exactly into Buffer.
void buffer_pack(Buffer* b);

// Append a single byte to current contents of Buffer.
void buffer_append_byte(Buffer* b, Byte u);

// Append a string of given length to current contents of Buffer.
// If len < 0, use null terminator, otherwise copy len bytes
void buffer_append_string(Buffer* b, const char* str, int len);

// Append a formatted signed / unsigned integer to current contents of Buffer.
void buffer_format_signed(Buffer* b, long long l);
void buffer_format_unsigned(Buffer* b, unsigned long long l);

// Append a formatted double to current contents of Buffer.
void buffer_format_double(Buffer* b, double d);

// Append a printf-formatted string to current contents of Buffer.
// THESE ARE EXPENSIVE.
void buffer_format_print(Buffer* b, const char* fmt, ...);
void buffer_format_vprint(Buffer* b, const char* fmt, va_list ap);

#endif
