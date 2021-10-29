#include "memory.h"
#include "wedge.h"

void build_from_slice(Wedge* w, Slice s, int null_terminated);

void wedge_build_from_char(Wedge* w, const char chr) {
    // NOT null terminated!
    char tmp[2] = { chr, 0x7f };
    Slice s = slice_from_memory(tmp, 1);
    build_from_slice(w, s, 0);
}

void wedge_build_from_string(Wedge* w, const char* str) {
    // ALWAYS null terminated!
    Slice s = slice_from_string(str, 0);
    build_from_slice(w, s, 1);
}

void wedge_build_from_ptr_len(Wedge* w, const char* ptr, uint32_t len) {
    // MAYBE null terminated?
    Slice s = slice_from_memory(ptr, len);
    wedge_build_from_slice(w, s);
}

void wedge_build_from_slice(Wedge* w, Slice s) {
    // MAYBE null terminated?
    int null_terminated = s.ptr[s.len] == '\0';
    build_from_slice(w, s, null_terminated);
}

void wedge_build_from_buffer(Wedge* w, Buffer* b) {
    // MAYBE null terminated?
    int null_terminated = 0;
    Slice s = buffer_slice(b);
    if (s.ptr[s.len-1] == '\0') {
        --s.len;
        null_terminated = 1;
    }
    build_from_slice(w, s, null_terminated);
}

void wedge_destroy(Wedge* w) {
    if (w->b) {
        buffer_release(w->b);
    }
}

void build_from_slice(Wedge* w, Slice s, int null_terminated) {
    if (null_terminated) {
        // Slice is null terminated -- keep just a slice copy
        w->b = 0;
        w->s = s;
    } else {
        // Slice is not null terminated -- build our own copy with the null
        w->b = buffer_allocate();
        buffer_append_slice(w->b, s);
        buffer_null_terminate(w->b);
        // Must fake slice's length to not include null
        w->s = slice_from_memory(w->b->ptr, s.len);
    }
}
