#ifndef SLICE_H_
#define SLICE_H_

/*
 * Slice -- a purely value type
 * read-only access to an array of bytes.
 * It does NOT assume data has a null terminator at the end.
 * Do NOT use with C standard strXXX() functions.
 * Small enough to be passed around by copying, not as a pointer.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Slice {
    const uint8_t* ptr; // pointer to beginning of data
    size_t len;         // length of data
} Slice;

// "context" when calling functions to tokenize / split
typedef struct SliceLookup {
    uint8_t map[256];
    Slice res;
} SliceLookup;

// The single null Slice.
extern Slice SLICE_NULL;

// Return true if Slice is null (invalid ptr).
#define slice_is_null(s) ((s).ptr == 0)

// Return true if Slice is empty (invalid ptr OR zero len).
#define slice_is_empty(s) ((s).ptr == 0 || (s).len == 0)

// Slice constructor from a string (const char*).
// Computes the length using strlen(), so string MUST be null-terminated.
Slice slice_build_from_string(const char* string);

// Slice constructor from a pointer and a length.
// Pointed data doesn't have to be null-terminated.
Slice slice_build_from_ptr_len(const uint8_t* ptr, size_t len);


/*
 * Slice algorithms.
 * These are all fully reentrant.
 */

// Compare two Slices, returning: l < r: -1; l > r: 1; l == r: 0
int slice_compare(Slice l, Slice r);

// Find byte in Slice.
// Return new Slice from byte found to the end of the given Slice.
// Return SLICE_NULL if not found.
Slice slice_find_byte(Slice s, uint8_t t);

// Find Slice in Slice.
// Return SLICE_NULL if not found.
Slice slice_find_slice(Slice s, Slice t);

// Tokenize Slice by repeatedly searching for bytes in separators.
// Return true if token was found, false otherwise.
// Return each token in lookup.res; only valid when true was returned.
// Intended to be used like this:
//
//   SliceLookup lookup = {0};
//   while (slice_tokenize(src, sep, &lookup)) {
//     // do something with lookup.res
//   }
bool slice_tokenize(Slice src, Slice sep, SliceLookup* lookup);

// Find longest span at beginning of Slice src with characters included in set.
// Return true if span exists, false otherwise.
// Return each span in lookup.res; only valid when true was returned.
// Intended to be used like this:
//
//   SliceLookup lookup = {0};
//   while (slice_split_included(src, set, &lookup)) {
//     // do something with lookup.res
//   }
bool slice_split_included(Slice src, Slice set, SliceLookup* lookup);

// Find longest span at beginning of Slice src with characters excluded from set.
// Return true if span exists, false otherwise.
// Return each span in lookup.res; only valid when true was returned.
// Intended to be used like this:
//
//   SliceLookup lookup = {0};
//   while (slice_split_excluded(src, set, &lookup)) {
//     // do something with lookup.res
//   }
bool slice_split_excluded(Slice src, Slice set, SliceLookup* lookup);

#endif
