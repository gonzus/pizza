#ifndef SLICE_H_
#define SLICE_H_

/*
 * Slice -- a purely value type
 * read-only access to an array of bytes.
 * It can be passed around by value (and of course by reference).
 * It does NOT allocate / release any memory, ever.
 * It does NOT assume data has a null terminator at the end.
 * Do NOT use with C standard strXXX() functions.
 * Small enough to be passed around by copying, not as a pointer.
 */

#include <stdbool.h>
#include <stdint.h>

/*
 * I would LOVE to use `uint8_t` as a `Byte`.  However, it is impractical
 * because so many things in C want to naturally just deal with `char` (not
 * even `unsigned char`), so going this route will either raise lots of
 * warnings, or it will force to compile with fewer warnings.
 *
 * typedef uint8_t Byte;
 */

typedef struct Slice {
    const char* ptr; // pointer to beginning of data
    uint32_t len;    // length of data
} Slice;

// "context" when calling functions to tokenize
typedef struct SliceLookup {
    char map[256];
    Slice result;
} SliceLookup;

// Return true if Slice is empty (invalid ptr OR zero len).
#define slice_empty(s) ((s).ptr == 0 || (s).len == 0)

// Compare two Slices; they are equal if they have the same length and
// slice_compare() returns 0 for them.
#define slice_equal(l, r) (((l).len == (r).len) && (slice_compare(l, r) == 0))

// Slice constructor from a string (const char*).
// If len < 0, use null terminator, otherwise copy len bytes.
Slice slice_from_string(const char* str, int len);

// Slice constructor from a pointer and a length.
// Pointed data doesn't have to be null-terminated.
Slice slice_from_memory(const char* ptr, uint32_t len);


/*
 * Slice algorithms.
 * These are all fully reentrant.
 */

bool slice_begins_with(Slice s, Slice w);

Slice slice_advance(Slice s, uint32_t len);
Slice slice_retract(Slice s, uint32_t len);

// Trim white space from beginning and end of slice.
Slice slice_trim(Slice s);

// Convert a slice to an integer.
// Return 0 if conversion is not possible, 1 otherwise.
int slice_integer(Slice s, long int* val);

// Convert a slice to a real.
// Return 0 if conversion is not possible, 1 otherwise.
int slice_real(Slice s, double* val);

// Compare two Slices, returning: l < r: -1; l > r: 1; l == r: 0
int slice_compare(Slice l, Slice r);

// Find byte in Slice.
// Return an empty slice if not found.
Slice slice_find_byte(Slice s, char t);

// Find Slice in Slice.
// Return an empty slice if not found.
Slice slice_find_slice(Slice s, Slice t);

// Tokenize Slice by repeatedly searching for any character in a separator slice.
// Return true if separator was found, false otherwise.
// Return each token in lookup.result; only valid when true was returned.
// Intended to be used like this:
//
//   Slice sep = slice_from_string(":", 0);
//   SliceLookup lookup = {0};
//   while (slice_tokenize_by_slice(src, sep, &lookup)) {
//     // do something with lookup.result
//   }
bool slice_tokenize_by_slice(Slice src, Slice sep, SliceLookup* lookup);

// Tokenize Slice by repeatedly searching for a separator character.
// Return true if separator was found, false otherwise.
// Return each token in lookup.result; only valid when true was returned.
// Intended to be used like this:
//
//   SliceLookup lookup = {0};
//   while (slice_tokenize_by_byte(src, ':', &lookup)) {
//     // do something with lookup.result
//   }
bool slice_tokenize_by_byte(Slice src, char t, SliceLookup* lookup);

// Split a slice on a given character, into left and right parts.
// Return true if character was found, false otherwise.
// When searching left to right:
// if not found, left = slice, right = empty
// When searching right to left.
// if not found, left = empty, right = slice
int slice_split_by_byte_l2r(Slice s, char t, Slice* l, Slice* r);
int slice_split_by_byte_r2l(Slice s, char t, Slice* l, Slice* r);

#endif
