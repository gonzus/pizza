#include "utf8.h"

Byte* utf8_decode(Byte* buf, Rune* r) {
    Byte* next;

    if (buf[0] < 0x80) {
        // length 1, simple ASCII
        *r = buf[0];
        next = buf + 1;
    } else if ((buf[0] & 0xe0) == 0xc0) {
        // length 2
        *r = ((Rune)(buf[0] & 0x1f) <<  6) |
             ((Rune)(buf[1] & 0x3f) <<  0);
        next = buf + 2;
    } else if ((buf[0] & 0xf0) == 0xe0) {
        // length 3
        *r = ((Rune)(buf[0] & 0x0f) << 12) |
             ((Rune)(buf[1] & 0x3f) <<  6) |
             ((Rune)(buf[2] & 0x3f) <<  0);
        next = buf + 3;
    } else if ((buf[0] & 0xf8) == 0xf0 && (buf[0] <= 0xf4)) {
        // length 4 and valid
        *r = ((Rune)(buf[0] & 0x07) << 18) |
             ((Rune)(buf[1] & 0x3f) << 12) |
             ((Rune)(buf[2] & 0x3f) <<  6) |
             ((Rune)(buf[3] & 0x3f) <<  0);
        next = buf + 4;
    } else {
        // invalid, skip this byte
        *r = -1;
        next = buf + 1;
    }

    if (*r >= 0xd800 && *r <= 0xdfff) {
        // invalid, surrogate half, byte(s) already skipped
        *r = -1;
    }

    return next;
}

Byte utf8_encode(Byte* buf, Rune r) {
    if (r <= 0x7f) {
        // length 1, simple ASCII
        buf[0] = (Byte) r;
        return 1;
    }
    if (r <= 0x07ff) {
        // length 2
        buf[0] = (Byte) (((r >> 6) & 0x1f) | 0xc0);
        buf[1] = (Byte) (((r >> 0) & 0x3f) | 0x80);
        return 2;
    }
    if (r <= 0xffff) {
        // length 3
        buf[0] = (Byte) (((r >> 12) & 0x0f) | 0xe0);
        buf[1] = (Byte) (((r >>  6) & 0x3f) | 0x80);
        buf[2] = (Byte) (((r >>  0) & 0x3f) | 0x80);
        return 3;
    }
    if (r <= 0x10ffff) {
        // length 4 and valid
        buf[0] = (Byte) (((r >> 18) & 0x07) | 0xf0);
        buf[1] = (Byte) (((r >> 12) & 0x3f) | 0x80);
        buf[2] = (Byte) (((r >>  6) & 0x3f) | 0x80);
        buf[3] = (Byte) (((r >>  0) & 0x3f) | 0x80);
        return 4;
    }

    // invalid - use replacement character
    buf[0] = (Byte) 0xef;
    buf[1] = (Byte) 0xbf;
    buf[2] = (Byte) 0xbd;
    return 3;
}
