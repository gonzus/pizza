#ifndef WEDGE_H_
#define WEDGE_H_

/*
 * A Wedge is a Slice but which guarantes a null-terminator.
 * Needed for the sad case of C libraries that expect a null-terminated string.
 */

#include "buffer.h"

typedef struct Wedge {
    Slice s;
    Buffer b;
} Wedge;

void wedge_build_from_string(Wedge* w, const char* str);
void wedge_build_from_char(Wedge* w, const char chr);
void wedge_build_from_ptr_len(Wedge* w, const char* ptr, uint32_t len);
void wedge_build_from_buffer(Wedge* w, Buffer* b);
void wedge_build_from_slice(Wedge* w, Slice s);

void wedge_destroy(Wedge* w);

#endif
