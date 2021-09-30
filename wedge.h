#ifndef WEDGE_H_
#define WEDGE_H_

#include "slice.h"
#include "buffer.h"

// MAYBE
// s always points to the null-terminated string
// b gets built and keeps its own copy if the string is not null-terminated; s would point to b in this case
//
// MAYBE
// we should always create b, because it is safer to keep our internal copy?
// needed for char anyway
typedef struct Wedge {
    Slice s;
    Buffer* b;
} Wedge;

void wedge_build_from_string(Wedge* w, const char* str);
void wedge_build_from_char(Wedge* w, const char chr);
void wedge_build_from_ptr_len(Wedge* w, const char* ptr, uint32_t len);
void wedge_build_from_buffer(Wedge* w, Buffer* b);
void wedge_build_from_slice(Wedge* w, Slice s);

void wedge_destroy(Wedge* w);

// TODO
// Use https://www.fileformat.info/info/unicode/char/fffd/index.htm
// to indicate errors in UTF-8 decoding

// TODO
// Where possible, change buffer allocate / release by build / destroy (ex: tests)
#endif
