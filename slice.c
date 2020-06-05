#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "log.h"
#include "slice.h"

Slice SLICE_NULL = { .ptr = 0, .len = 0 };

Slice slice_wrap_ptr_len(const Byte* ptr, Size len) {
    Slice s = { .ptr = ptr, .len = len };
    return s;
}

Slice slice_wrap_string(const char* string) {
    return slice_wrap_ptr_len((const Byte*) string, string == 0 ? 0 : strlen(string));
}

int slice_compare(Slice l, Slice r) {
    for (Size j = 0; 1; ++j) {
        if (j >= l.len && j >= r.len) {
            return 0;
        }
        if (j >= l.len) {
            return -1;
        }
        if (j >= r.len) {
            return +1;
        }
        if (l.ptr[j] < r.ptr[j]) {
            return -1;
        }
        if (l.ptr[j] > r.ptr[j]) {
            return +1;
        }
    }
    return 0;
}

Slice slice_find_byte(Slice s, Byte t) {
    Size j = 0;
    Size top = s.len;
    for (j = 0; j < top; ++j) {
        if (s.ptr[j] == t) {
            break;
        }
    }
    if (j < top) {
        return slice_wrap_ptr_len(s.ptr + j, 1);
    }
    return SLICE_NULL;
}

Slice slice_find_slice(Slice s, Slice t) {
    if (s.len < t.len) {
        return SLICE_NULL;
    }

#if defined(_GNU_SOURCE)
    Byte* p = memmem(s.ptr, s.len, t.ptr, t.len);
    if (p) {
        return slice_wrap_ptr_len(p, t.len);
    }
#else
    // TODO: implement a good search algorihtm here?
    Size j = 0;
    Size top = s.len - t.len + 1;
    for (j = 0; j < top; ++j) {
        if (memcmp(s.ptr + j, t.ptr, t.len) == 0) {
            break;
        }
    }
    if (j < top) {
        return slice_wrap_ptr_len(s.ptr + j, t.len);
    }
#endif

    return SLICE_NULL;
}

static void set_bitmap(Byte* map, Size len, Slice set) {
    memset(map, 0, len);
    for (Size j = 0; j < set.len; ++j) {
        map[set.ptr[j]] = 1;
    }
}

bool slice_tokenize(Slice src, Slice sep, Slice* tok) {
    Size start = 0;
    if (tok->ptr) {
        start = (tok->ptr - src.ptr) + tok->len + 1;
        tok->ptr = 0;
    }

    if (start >= src.len) {
        return 0;
    }

    // TODO: keep map between calls -- how?
    Byte map[256];
    set_bitmap(map, 256, sep);

    const Byte* p = src.ptr + start;
    Size l = src.len - start;

    // skip separators
    Size j = 0;
    for (; j < l; ++j) {
        if (!map[p[j]]) {
            break;
        }
    }
    if (j >= l) {
        return 0;
    }

    // find end of token
    Size k = j;
    for (; k < l; ++k) {
        if (map[p[k]]) {
            break;
        }
    }
    tok->ptr = p + j;
    tok->len = (k < l) ? (k-j) : (l-j);
    return 1;
}

bool slice_split(Slice src, bool inc, Slice set, Slice* l, Slice* r) {
    Byte map[256];
    set_bitmap(map, 256, set);

    Size j = 0;
    for (j = 0; j < src.len; ++j) {
        bool match = map[src.ptr[j]];
        if (inc ? !match : match) {
            break;
        }
    }
    if (l) {
        l->ptr = src.ptr;
        l->len = j;
    }
    if (r) {
        r->ptr = src.ptr + j;
        r->len = src.len - j;
    }
    return j;
}

bool slice_split_included(Slice src, Slice set, Slice* l, Slice* r) {
    return slice_split(src, 1, set, l, r);
}

bool slice_split_excluded(Slice src, Slice set, Slice* l, Slice* r) {
    return slice_split(src, 0, set, l, r);
}
