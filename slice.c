#include <string.h>
#include "slice.h"

Slice SLICE_NULL = { .ptr = 0, .len = 0 };

static bool slice_split(Slice src, bool inc, Slice set, SliceLookup* lookup);
static void lookup_init(SliceLookup* lookup, Slice src, Slice set);

Slice slice_build_from_ptr_len(const uint8_t* ptr, size_t len) {
    Slice s = { .ptr = ptr, .len = len };
    return s;
}

Slice slice_build_from_string(const char* string) {
    int len = string == 0 ? 0 : strlen(string);
    return slice_build_from_ptr_len((const uint8_t*) string, len);
}

int slice_compare(Slice l, Slice r) {
    for (size_t j = 0; 1; ++j) {
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
        // they are equal so far, must keep looking
    }
    return 0;
}

Slice slice_find_byte(Slice s, uint8_t t) {
    size_t j = 0;
    for (j = 0; j < s.len; ++j) {
        if (s.ptr[j] == t) {
            break;
        }
    }
    if (j < s.len) {
        return slice_build_from_ptr_len(s.ptr + j, s.len - j);
    }
    return SLICE_NULL;
}

Slice slice_find_slice(Slice s, Slice t) {
    if (s.len < t.len) {
        return SLICE_NULL;
    }

#if defined(_GNU_SOURCE)
    uint8_t* p = memmem(s.ptr, s.len, t.ptr, t.len);
    if (p) {
        return slice_build_from_ptr_len(p, t.len);
    }
#else
    // TODO: implement a good search algorihtm here?
    size_t j = 0;
    size_t top = s.len - t.len + 1;
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

bool slice_tokenize(Slice src, Slice sep, SliceLookup* lookup) {
    bool first = !lookup->res.ptr;
    size_t start = 0;
    if (!first) {
        // not the first time we were called -- we already found a previous
        // token, and stopped at a separator.
        start = (lookup->res.ptr - src.ptr) + lookup->res.len + 1;
    }

    if (start >= src.len) {
        return false; // no bytes left
    }

    if (first) {
        // first time we are called -- initialize lookup
        lookup_init(lookup, src, sep);
    }

    // current position and remaining length
    const uint8_t* p = src.ptr + start;
    size_t l = src.len - start;

    // skip all separators
    size_t j = 0;
    for (; j < l; ++j) {
        if (!lookup->map[p[j]]) {
            break;
        }
    }
    if (j >= l) {
        return false; // no bytes left
    }

    // we are looking at a token -- find separator after it
    size_t k = j;
    for (; k < l; ++k) {
        if (lookup->map[p[k]]) {
            break;
        }
    }

    // remember where separator was
    lookup->res = slice_build_from_ptr_len(p + j, (k < l) ? (k-j) : (l-j));

    return true; // found next token
}

bool slice_split_included(Slice src, Slice set, SliceLookup* lookup) {
    return slice_split(src, true, set, lookup);
}

bool slice_split_excluded(Slice src, Slice set, SliceLookup* lookup) {
    return slice_split(src, false, set, lookup);
}

static bool slice_split(Slice src, bool inc, Slice set, SliceLookup* lookup) {
    bool first = !lookup->res.ptr;
    size_t start = 0;
    if (!first) {
        // not the first time we were called -- we already found a previous
        // span, and stopped at a non-span character.
        start = (lookup->res.ptr - src.ptr) + lookup->res.len + 1;
    }

    if (start >= src.len) {
        return false; // no bytes left
    }

    if (first) {
        // first time we are called -- initialize lookup
        lookup_init(lookup, src, set);
    }

    // current position and remaining length
    const uint8_t* p = src.ptr + start;
    size_t l = src.len - start;

    size_t j = 0;
    if (!first) {
        // skip all non-span characters
        for (; j < l; ++j) {
            bool match = lookup->map[src.ptr[j]];
            if (inc ? match : !match) {
                break;
            }
        }
        if (j >= l) {
            return false; // no bytes left
        }
    }

    // we are looking at a span -- find non-span character after it
    size_t k = j;
    for (; k < l; ++k) {
        bool match = lookup->map[src.ptr[k]];
        if (inc ? !match : match) {
            break;
        }
    }

    // remember where non-span character was
    lookup->res = slice_build_from_ptr_len(p + j, (k < l) ? (k-j) : (l-j));

    return true; // found next span
}

static void lookup_init(SliceLookup* lookup, Slice src, Slice set) {
    lookup->res = slice_build_from_ptr_len(src.ptr, 0);
    memset(lookup->map, 0, 256);
    for (size_t j = 0; j < set.len; ++j) {
        lookup->map[set.ptr[j]] = 1;
    }
}
