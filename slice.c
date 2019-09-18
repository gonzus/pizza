#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "log.h"
#include "slice.h"

Slice SLICE_NULL = { .ptr = 0, .len = 0 };

static void slice_dump_file(Slice s, FILE* fp);

bool slice_is_null(Slice s) {
    return s.ptr == 0;
}

bool slice_is_empty(Slice s) {
    return s.ptr != 0 && s.len == 0;
}

Size slice_get_length(Slice s) {
    return s.len;
}

Slice slice_wrap_ptr_len(const Byte* ptr, Size len) {
    Slice s = { .ptr = ptr, .len = len };
    return s;
}

Slice slice_wrap_string(const char* string) {
    return slice_wrap_ptr_len((const Byte*) string, string == 0 ? 0 : strlen(string));
}

void slice_dump(Slice s) {
    slice_dump_file(s, stderr);
}

bool slice_compare(Slice l, Slice r) {
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
    return SLICE_NULL;
}

static void set_bitmap(Byte* map, Size len, Slice set) {
    memset(map, 0, len);
    for (Size j = 0; j < set.len; ++j) {
        map[set.ptr[j]] = 1;
    }
}

// TODO: keep map between calls -- how?
bool slice_tokenize(Slice s, Slice sep, Slice* token) {
    Size start = 0;
    if (token->ptr) {
        start = (token->ptr - s.ptr) + token->len + 1;
        token->ptr = 0;
    }

    if (start >= s.len) {
        return 0;
    }

    Byte map[256];
    set_bitmap(map, 256, sep);

    const Byte* p = s.ptr + start;
    Size l = s.len - start;
    Size j = 0;
    for (j = 0; j < l; ++j) {
        if (map[p[j]]) {
            break;
        }
    }

    token->ptr = p;
    if (j < l) {
        token->len = j;
    } else {
        token->len = l;
    }
    return 1;
}

bool slice_split(Slice s, bool included, Slice set, Slice* l, Slice* r) {
    Byte map[256];
    set_bitmap(map, 256, set);

    Size j = 0;
    for (j = 0; j < s.len; ++j) {
        bool match = map[s.ptr[j]];
        if (included ? !match : match) {
            break;
        }
    }
    if (l) {
        l->ptr = s.ptr;
        l->len = j;
    }
    if (r) {
        r->ptr = s.ptr + j;
        r->len = s.len - j;
    }
    return j;
}

bool slice_split_included(Slice s, Slice set, Slice* l, Slice* r) {
    return slice_split(s, 1, set, l, r);
}

bool slice_split_excluded(Slice s, Slice set, Slice* l, Slice* r) {
    return slice_split(s, 0, set, l, r);
}

static void dump_line(Size row, const char* byte, Size white, const char* text, FILE* fp) {
    fprintf(fp, "%06x | %s%*s | %-16s |\n", row, byte, white, "", text);
}

static void slice_dump_file(Slice s, FILE* fp) {
    char byte[16*3+1];
    Size bpos = 0;
    char text[16+1];
    Size dpos = 0;
    Size row = 0;
    Size col = 0;
    for (Size j = 0; j < s.len; ++j) {
        unsigned char uc = (unsigned char) s.ptr[j];
        Size ui = (Size) uc;
        bpos += sprintf(byte + bpos, "%s%02x", col ? " " : "", ui);
        dpos += sprintf(text + dpos, "%c", isprint(uc) ? uc : '.');
        col++;
        if (col == 16) {
            dump_line(row, byte, 0, text, fp);
            col = bpos = dpos = 0;
            row += 16;
        }
    }
    if (dpos > 0) {
        dump_line(row, byte, (16-col)*3, text, fp);
    }
}
