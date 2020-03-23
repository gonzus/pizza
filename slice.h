#ifndef SLICE_H_
#define SLICE_H_

/*
 * Slice -- read-only access to an array of bytes.
 * It does NOT assume data has a null terminator at the end.
 * Do NOT use with C standard strXXX() functions.
 * Small enough to be passed around by copying, not as a pointer.
 */

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t  Byte;  // type for bytes (0..255 = 2^8-1)
typedef uint32_t Size;  // type for sizes (0..2^32-1)

typedef struct Slice {
    const Byte* ptr;    // pointer to beginning of data
    Size len;           // length of data
} Slice;

// The null Slice.
extern Slice SLICE_NULL;

// Return true if Slice is null (no ptr).
#define slice_is_null(s) ((s).ptr == 0)

// Return true if Slice is empty (valid ptr, zero len).
#define slice_is_empty(s) ((s).ptr != 0 && (s).len == 0)

// Return Slice pointer.
#define slice_get_ptr(s) ((s).ptr)

// Return Slice length.
#define slice_get_len(s) ((s).len)

// Wrap a string (const char*) into a Slice.
// Computes the length using strlen(), so string MUST be null-terminated.
Slice slice_wrap_string(const char* string);

// Wrap a given number of bytes from an array of Bytes (or a const char*) into a Slice.
// String doesn't have to be null-terminated.
Slice slice_wrap_ptr_len(const Byte* ptr, Size len);

// Dump a Slice to stderr.
void slice_dump(Slice s);


/*
 * Slice algorithms.
 * These are all fully reentrant.
 */

// Compare two Slices, returning: l < r: -1; l > r: 1; l == r: 0
int slice_compare(Slice l, Slice r);

// Find byte in Slice.
// Return SLICE_NULL if not found.
Slice slice_find_byte(Slice s, Byte t);

// Find Slice in Slice.
// Return SLICE_NULL if not found.
Slice slice_find_slice(Slice s, Slice t);

// Tokenize Slice by repeatedly searching for bytes in separators.
// Return true if token was found, false otherwise.
// Return each found token in tok.
// Intended to be used like this:
//
//   for (Slice tok = SLICE_NULL; slice_tokenize(src, sep, &tok); ) {
//     // do something with tok
//   }
bool slice_tokenize(Slice src, Slice sep, Slice* tok);

// Find longest span at beginning of Slice src with characters included in / excluded from set.
// Returns left and right Slices.
bool slice_split(Slice src, bool inc, Slice set, Slice* l, Slice* r);

// Find longest span at beginning of Slice src with characters included in set.
// Returns left and right Slices.
// Calls slice_split() with inc=true.
bool slice_split_included(Slice src, Slice set, Slice* l, Slice* r);

// Find longest span at beginning of Slice src with characters excluded from set.
// Returns left and right Slices.
// Calls slice_split() with inc=false.
bool slice_split_excluded(Slice src, Slice set, Slice* l, Slice* r);

#endif
