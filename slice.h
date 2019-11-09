#ifndef SLICE_H_
#define SLICE_H_

/*
 * Slice -- read-only access to an array of bytes
 * it does NOT assume data has a null terminator at the end
 * do NOT use with C standard strXXX() functions
 * small enough to be passed around by copying, not with a reference (pointer)
 */

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t  Byte;
typedef uint16_t Size;

typedef struct Slice {
    const Byte* ptr;    // pointer to beginning of data
    Size len;           // length of data
} Slice;

// the null Slice
extern Slice SLICE_NULL;

// return true if Slice is null (no ptr)
bool slice_is_null(Slice s);

// return true if Slice is empty (valid ptr, zero len)
bool slice_is_empty(Slice s);

// return Slice length
Size slice_get_length(Slice s);

// wrap a string (const char*) into a Slice
// computes the length using strlen(), so string must be null-terminated
Slice slice_wrap_string(const char* string);

// wrap a given number of bytes from an array of Bytes (or a const char*) into a Slice.
// string doesn't have to be null-terminated
Slice slice_wrap_ptr_len(const Byte* ptr, Size len);

// dump a Slice into stderr
void slice_dump(Slice s);


/*
 * Slice algorithms
 */

// compare two Slices, returning: l < r: -1; l > r: 1; l == r: 0
bool slice_compare(Slice l, Slice r);

// find byte in Slice
Slice slice_find_byte(Slice s, Byte t);

// find Slice in Slice
Slice slice_find_slice(Slice s, Slice t);

/*
 * tokenize Slice s by repeatedly searching for bytes in separators; return each token
 * intended to be used like this:
 *
 *    for (Slice token = SLICE_NULL; slice_tokenize(s, separators, &token); ) {
 *        // do something with token
 *    }
 */
bool slice_tokenize(Slice s, Slice separators, Slice* token);

/*
 * find longest span at beginning of Slice with characters included in / excluded from set
 * returns left and right Slices
 */
bool slice_split(Slice s, bool included, Slice set, Slice* l, Slice* r);

/*
 * find longest span at beginning of Slice with characters included in set
 * returns left and right Slices
 * calls slice_split with included=1
 */
bool slice_split_included(Slice s, Slice set, Slice* l, Slice* r);

/*
 * find longest span at beginning of Slice with characters excluded from set
 * returns left and right Slices
 * calls slice_split with included=0
 */
bool slice_split_excluded(Slice s, Slice set, Slice* l, Slice* r);

#endif
