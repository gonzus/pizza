#include "pizza/base64.h"

static const unsigned char enc_64[256] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};
#define E64(c) (uint8_t)enc_64[(int)c]

static const char dec_64[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
#define D64(c) (uint8_t)dec_64[(int)c]

#define B64DEC1(s, p) (uint8_t)((E64(s.ptr[p+0]) << 2) | (E64(s.ptr[p+1]) >> 4))
#define B64DEC2(s, p) (uint8_t)((E64(s.ptr[p+1]) << 4) | (E64(s.ptr[p+2]) >> 2))
#define B64DEC3(s, p) (uint8_t)((E64(s.ptr[p+2]) << 6) | (E64(s.ptr[p+3]) >> 0))

int base64_decode(Slice encoded, Buffer *decoded) {
    uint32_t left = 0;
    while (left < encoded.len && E64(encoded.ptr[left]) < 64) {
        ++left;
    }

    int orig = decoded->len;
    uint32_t pos = 0;
    while (left > 4) {
        buffer_append_byte(decoded, B64DEC1(encoded, pos));
        buffer_append_byte(decoded, B64DEC2(encoded, pos));
        buffer_append_byte(decoded, B64DEC3(encoded, pos));
        pos += 4;
        left -= 4;
    }

    if (left > 1) {
        buffer_append_byte(decoded, B64DEC1(encoded, pos));
    }
    if (left > 2) {
        buffer_append_byte(decoded, B64DEC2(encoded, pos));
    }
    if (left > 3) {
        buffer_append_byte(decoded, B64DEC3(encoded, pos));
    }

    return decoded->len - orig;
}

#define B64PAD '='
#define B64ENC1x(s, p) ((s.ptr[p+0] >> 2) & 0x3F)
#define B64ENC2a(s, p) ((s.ptr[p+0] & 0x03) << 4)
#define B64ENC2b(s, p) ((s.ptr[p+1] & 0xF0) >> 4)
#define B64ENC3a(s, p) ((s.ptr[p+1] & 0x0F) << 2)
#define B64ENC3b(s, p) ((s.ptr[p+2] & 0xC0) >> 6)
#define B64ENC4x(s, p) ((s.ptr[p+2] & 0x3F))

int base64_encode(Slice decoded, Buffer *encoded) {
    int orig = encoded->len;
    uint32_t pos = 0;
    while (pos < (decoded.len - 2)) {
        buffer_append_byte(encoded, D64(B64ENC1x(decoded, pos)));
        buffer_append_byte(encoded, D64(B64ENC2a(decoded, pos) | B64ENC2b(decoded, pos)));
        buffer_append_byte(encoded, D64(B64ENC3a(decoded, pos) | B64ENC3b(decoded, pos)));
        buffer_append_byte(encoded, D64(B64ENC4x(decoded, pos)));
        pos += 3;
    }

    switch (decoded.len - pos) { // bytes left -- can be 0, 1 or 2
        case 0:
            break;
        case 1:
            buffer_append_byte(encoded, D64(B64ENC1x(decoded, pos)));
            buffer_append_byte(encoded, D64(B64ENC2a(decoded, pos)));
            buffer_append_byte(encoded, B64PAD);
            buffer_append_byte(encoded, B64PAD);
            break;
        case 2:
            buffer_append_byte(encoded, D64(B64ENC1x(decoded, pos)));
            buffer_append_byte(encoded, D64(B64ENC2a(decoded, pos) | B64ENC2b(decoded, pos)));
            buffer_append_byte(encoded, D64(B64ENC3a(decoded, pos)));
            buffer_append_byte(encoded, B64PAD);
            break;
    }

    return encoded->len - orig;
}
