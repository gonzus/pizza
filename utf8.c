#include "utf8.h"

// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

static const uint8_t utf8d[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
    8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
    0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
    0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
    0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
    1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
    1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
    1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
};

static uint32_t decode(uint32_t* state, uint32_t* rune, uint8_t byte) {
    uint8_t type = utf8d[byte];

    *rune = *state
          ? (byte & 0x3fu) | (*rune << 6)
          : (0xff >> type) & (byte);
    *state = utf8d[256 + *state * 16 + type];
    return *state;
}

uint32_t utf8_decode(Slice* s) {
    uint32_t state = 0;
    uint32_t len = 0;
    uint32_t rune = 0;
    for (len = 0; len < s->len; ++len) {
        if (!decode(&state, &rune, s->ptr[len])) {
            break;
        }
    }
    if (state) {
        rune = UTF8_INVALID_RUNE;
    } else {
        ++len;
    }
    s->ptr += len;
    s->len -= len;
    return rune;
}

unsigned int utf8_encode(uint32_t rune, Buffer* b) {
    if (rune <= 0x7f) {
        // length 1, simple ASCII
        buffer_append_byte(b, (uint8_t) rune);
        return 1;
    }

    if (rune <= 0x07ff) {
        // length 2
        buffer_append_byte(b, (uint8_t) (((rune >> 6) & 0x1f) | 0xc0));
        buffer_append_byte(b, (uint8_t) (((rune >> 0) & 0x3f) | 0x80));
        return 2;
    }

    if (rune <= 0xffff) {
        // length 3
        buffer_append_byte(b, (uint8_t) (((rune >> 12) & 0x0f) | 0xe0));
        buffer_append_byte(b, (uint8_t) (((rune >>  6) & 0x3f) | 0x80));
        buffer_append_byte(b, (uint8_t) (((rune >>  0) & 0x3f) | 0x80));
        return 3;
    }

    if (rune <= 0x10ffff) {
        // length 4 and valid
        buffer_append_byte(b, (uint8_t) (((rune >> 18) & 0x07) | 0xf0));
        buffer_append_byte(b, (uint8_t) (((rune >> 12) & 0x3f) | 0x80));
        buffer_append_byte(b, (uint8_t) (((rune >>  6) & 0x3f) | 0x80));
        buffer_append_byte(b, (uint8_t) (((rune >>  0) & 0x3f) | 0x80));
        return 4;
    }

    // invalid -- use replacement character encoded as UTF-8
    buffer_append_byte(b, (uint8_t) 0xef);
    buffer_append_byte(b, (uint8_t) 0xbf);
    buffer_append_byte(b, (uint8_t) 0xbd);
    return 3;
}
