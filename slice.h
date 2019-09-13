#ifndef SLICE_H_
#define SLICE_H_

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

// wrap a given number of bytes from an array of Bytes into a slice.
slice slice_wrap_ptr_len(const Byte* ptr, unsigned int len);
/*
 * copy a slice into a string (char*) and return the buffer; includes a null terminator
 * the string buffer must already have the right size (slice.len + 1)
 */
unsigned int slice_to_string(slice s, char* string);

/*
 * tokenize slice s by repeatedly searching for bytes in sep, returning each token in c
 * intended to be used like this:
 *
 *    for (slice token = SLICE_NULL; slice_tokenize(s, sep, &token); ) {
 *        // do something with token
 *    }
 */
int slice_tokenize(slice s, slice sep, slice* c);

/*
 * find longest span at beginning of slice with characters included in / excluded from set
 * returns left and right slices
 */
int slice_split(slice s, int included, slice set, slice* l, slice* r);

/*
 * find longest span at beginning of slice with characters included in set
 * returns left and right slices
 * calls slice_split with included=1
 */
int slice_split_included(slice s, slice set, slice* l, slice* r);

/*
 * find longest span at beginning of slice with characters excluded from set
 * returns left and right slices
 * calls slice_split with included=0
 */
int slice_split_excluded(slice s, slice set, slice* l, slice* r);

// dump a slice into stderr
void slice_dump(slice s);

#endif
