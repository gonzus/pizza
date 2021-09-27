#include <string.h>
#include "slice.h"

Slice SLICE_NULL = { .ptr = 0, .len = 0 };

static void lookup_init(SliceLookup* lookup, Slice src, Slice set);

Slice slice_build_from_ptr_len(const char* ptr, uint32_t len) {
    Slice s = { .ptr = ptr, .len = len };
    return s;
}

Slice slice_build_from_string(const char* str) {
    return slice_build_from_ptr_len(str, str == 0 ? 0 : strlen(str));
}

int slice_compare(Slice l, Slice r) {
    for (uint32_t j = 0; 1; ++j) {
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

Slice slice_find_byte(Slice s, char t) {
    uint32_t j = 0;
    for (j = 0; j < s.len; ++j) {
        if (s.ptr[j] == t) {
            break;
        }
    }
    if (j < s.len) {
        return slice_build_from_ptr_len(s.ptr + j, 1);
    }
    return SLICE_NULL;
}

Slice slice_find_slice(Slice s, Slice t) {
    if (s.len < t.len) {
        return SLICE_NULL;
    }

#if defined(_GNU_SOURCE)
    const char* p = memmem(s.ptr, s.len, t.ptr, t.len);
    if (p) {
        return slice_build_from_ptr_len(p, t.len);
    }
#else
    // TODO: implement a good search algorithm here?
    uint32_t j = 0;
    uint32_t top = s.len - t.len + 1;
    for (j = 0; j < top; ++j) {
        if (memcmp(s.ptr + j, t.ptr, t.len) == 0) {
            break;
        }
    }
    if (j < top) {
        return slice_build_from_ptr_len(s.ptr + j, t.len);
    }
#endif

    return SLICE_NULL;
}

static bool slice_tokenize(Slice src, Slice s, SliceLookup* lookup) {
    bool first = !lookup->result.ptr;
    uint32_t start = 0;
    if (!first) {
        // not the first time we were called -- we already found a previous
        // token, and stopped at a separator.
        start = (lookup->result.ptr - src.ptr) + lookup->result.len + 1;
    }

    if (start >= src.len) {
        return false; // no bytes left
    }

    if (first) {
        // first time we are called -- initialize lookup
        lookup_init(lookup, src, s);
    }

    // current position and remaining length
    const char* p = src.ptr + start;
    uint32_t l = src.len - start;

    // skip all separators using our map
    uint32_t j = 0;
    for (; j < l; ++j) {
        if (!lookup->map[(int)p[j]]) {
            break;
        }
    }
    if (j >= l) {
        return false; // no bytes left
    }

    // we are looking at a token
    // find separator after token using our map
    uint32_t k = j;
    for (; k < l; ++k) {
        if (lookup->map[(int)p[k]]) {
            break;
        }
    }

    // produce current result
    lookup->result = slice_build_from_ptr_len(p + j, (k < l) ? (k-j) : (l-j));

    return true; // found next token
}

bool slice_tokenize_by_slice(Slice src, Slice s, SliceLookup* lookup) {
    return slice_tokenize(src, s, lookup);
}

bool slice_tokenize_by_byte(Slice src, char t, SliceLookup* lookup) {
    const char tmp[2] = { t, '\0' };
    return slice_tokenize(src, slice_build_from_ptr_len(tmp, 1), lookup);
}

int slice_split_by_byte_l2r(Slice s, char t, Slice* l, Slice* r) {
    for (int32_t j = 0; j < (int32_t) s.len; ++j) {
        if (s.ptr[j] == t) {
            *l = slice_build_from_ptr_len(s.ptr, j);
            *r = slice_build_from_ptr_len(s.ptr + j + 1, s.len - j - 1);
            return 1;
        }
    }
    *l = s;
    *r = slice_build_from_ptr_len(s.ptr + s.len, 0);
    return 0;
}

int slice_split_by_byte_r2l(Slice s, char t, Slice* l, Slice* r) {
    for (int32_t j = (int32_t) s.len - 1; j >= 0; --j) {
        if (s.ptr[j] == t) {
            *l = slice_build_from_ptr_len(s.ptr, j);
            *r = slice_build_from_ptr_len(s.ptr + j + 1, s.len - j - 1);
            return 1;
        }
    }
    *l = slice_build_from_ptr_len(s.ptr, 0);
    *r = s;
    return 0;
}

static void lookup_init(SliceLookup* lookup, Slice src, Slice set) {
    // reset result
    lookup->result = slice_build_from_ptr_len(src.ptr, 0);

    // create a quick map of the bytes / chars we are interested in
    memset(lookup->map, 0, 256);
    for (uint32_t j = 0; j < set.len; ++j) {
        lookup->map[(int)set.ptr[j]] = 1;
    }
}
