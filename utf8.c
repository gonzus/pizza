#include "utf8.h"

Rune utf8_decode(Slice s, Slice* rest) {
    Byte l = 0;
    Rune r = UTF8_INVALID_RUNE;
    if (s.len >= 1 && s.ptr[0] < 0x80) {
        // length 1, simple ASCII
        l = 1;
        r = (Rune) s.ptr[0];
    } else if (s.len >= 2 && (s.ptr[0] & 0xe0) == 0xc0) {
        // length 2
        l = 2;
        r = ((Rune)(s.ptr[0] & 0x1f) <<  6) |
            ((Rune)(s.ptr[1] & 0x3f) <<  0);
    } else if (s.len >= 3 && (s.ptr[0] & 0xf0) == 0xe0) {
        // length 3
        l = 3;
        r = ((Rune)(s.ptr[0] & 0x0f) << 12) |
            ((Rune)(s.ptr[1] & 0x3f) <<  6) |
            ((Rune)(s.ptr[2] & 0x3f) <<  0);
    } else if (s.len >= 4 && (s.ptr[0] & 0xf8) == 0xf0 && s.ptr[0] <= 0xf4) {
        // length 4 and valid
        l = 4;
        r = ((Rune)(s.ptr[0] & 0x07) << 18) |
            ((Rune)(s.ptr[1] & 0x3f) << 12) |
            ((Rune)(s.ptr[2] & 0x3f) <<  6) |
            ((Rune)(s.ptr[3] & 0x3f) <<  0);
    }

    if (r >= 0xd800 && r <= 0xdfff) {
        // invalid, surrogate half, byte(s) already skipped
        r = UTF8_INVALID_RUNE;
    }

    if (rest) {
        *rest = slice_wrap_ptr_len(s.ptr + l, s.len - l);
    }
    return r;
}

unsigned int utf8_encode(Rune r, Buffer* b) {
    if (r <= 0x7f) {
        // length 1, simple ASCII
        buffer_append_byte(b, (Byte) r);
        return 1;
    }

    if (r <= 0x07ff) {
        // length 2
        buffer_append_byte(b, (Byte) (((r >> 6) & 0x1f) | 0xc0));
        buffer_append_byte(b, (Byte) (((r >> 0) & 0x3f) | 0x80));
        return 2;
    }

    if (r <= 0xffff) {
        // length 3
        buffer_append_byte(b, (Byte) (((r >> 12) & 0x0f) | 0xe0));
        buffer_append_byte(b, (Byte) (((r >>  6) & 0x3f) | 0x80));
        buffer_append_byte(b, (Byte) (((r >>  0) & 0x3f) | 0x80));
        return 3;
    }

    if (r <= 0x10ffff) {
        // length 4 and valid
        buffer_append_byte(b, (Byte) (((r >> 18) & 0x07) | 0xf0));
        buffer_append_byte(b, (Byte) (((r >> 12) & 0x3f) | 0x80));
        buffer_append_byte(b, (Byte) (((r >>  6) & 0x3f) | 0x80));
        buffer_append_byte(b, (Byte) (((r >>  0) & 0x3f) | 0x80));
        return 4;
    }

    // invalid
    return 0;
}
