#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "log.h"
#include "slice.h"

slice SLICE_NULL = { .ptr = 0, .len = 0 };

static void slice_dump_file(slice s, FILE* fp);

int slice_is_null(slice s) {
    return s.ptr == 0;
}

int slice_is_empty(slice s) {
    return s.ptr != 0 && s.len == 0;
}

unsigned int slice_get_length(slice s) {
    return s.len;
}

slice slice_wrap_ptr_len(const Byte* ptr, unsigned int len) {
    slice s = { .ptr = ptr, .len = len };
    return s;
}

slice slice_wrap_string(const char* string) {
    return slice_wrap_ptr_len((const Byte*) string, string == 0 ? 0 : strlen(string));
}

slice slice_wrap_string_length(const char* string, unsigned int length) {
    return slice_wrap_ptr_len((const Byte*) string, length);
}

unsigned int slice_to_string(slice s, char* string) {
    memcpy(string, s.ptr, s.len);
    string[s.len] = '\0';
    return s.len;
}

void slice_dump(slice s) {
    slice_dump_file(s, stderr);
}

int slice_compare(slice l, slice r) {
    for (unsigned int j = 0; 1; ++j) {
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

slice slice_find_byte(slice s, Byte t) {
    unsigned int j = 0;
    unsigned int top = s.len;
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

slice slice_find_slice(slice s, slice t) {
    if (s.len < t.len) {
        return SLICE_NULL;
    }

    unsigned int j = 0;
    unsigned int top = s.len - t.len + 1;
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

static void set_bitmap(Byte* map, int len, slice set) {
    memset(map, 0, len);
    for (unsigned int j = 0; j < set.len; ++j) {
        map[set.ptr[j]] = 1;
    }
}

// TODO: keep map between calls -- how?
int slice_tokenize(slice s, slice sep, slice* token) {
    unsigned int start = 0;
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
    unsigned int l = s.len - start;
    unsigned int j = 0;
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

int slice_split(slice s, int included, slice set, slice* l, slice* r) {
    Byte map[256];
    set_bitmap(map, 256, set);

    unsigned int j = 0;
    for (j = 0; j < s.len; ++j) {
        int match = map[s.ptr[j]];
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

int slice_split_included(slice s, slice set, slice* l, slice* r) {
    return slice_split(s, 1, set, l, r);
}

int slice_split_excluded(slice s, slice set, slice* l, slice* r) {
    return slice_split(s, 0, set, l, r);
}

static void dump_line(unsigned int row, const char* byte, unsigned int white, const char* text, FILE* fp) {
    fprintf(fp, "%06x | %s%*s | %-16s |\n", row, byte, white, "", text);
}

static void slice_dump_file(slice s, FILE* fp) {
    char byte[16*3+1];
    int bpos = 0;
    char text[16+1];
    int dpos = 0;
    unsigned int row = 0;
    unsigned int col = 0;
    for (unsigned int j = 0; j < s.len; ++j) {
        unsigned char uc = (unsigned char) s.ptr[j];
        unsigned int ui = (unsigned int) uc;
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