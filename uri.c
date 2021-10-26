#include <ctype.h>
// #include <stdio.h>
// #include <string.h>
#include "uri.h"

#define NIBBLE_BITS 4
#define MAKE_BYTE(nh, nl) (((nh) << NIBBLE_BITS) | (nl))

/*
 * Table has a 0 if that character cannot be a hex digit;
 * otherwise it has the decimal value for that hex digit.
 */
static char uri_decode_tbl[256] = {
/*    0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f */
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  /* 0:   0 ~  15 */
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  /* 1:  16 ~  31 */
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  /* 2:  32 ~  47 */
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   0,   0,   0,   0,   0,   0,  /* 3:  48 ~  63 */
      0,  10,  11,  12,  13,  14,  15,   0,   0,   0,   0,   0,   0,   0,   0,   0,  /* 4:  64 ~  79 */
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  /* 5:  80 ~  95 */
      0,  10,  11,  12,  13,  14,  15,   0,   0,   0,   0,   0,   0,   0,   0,   0,  /* 6:  96 ~ 111 */
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  /* 7: 112 ~ 127 */
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  /* 8: 128 ~ 143 */
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  /* 9: 144 ~ 159 */
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  /* a: 160 ~ 175 */
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  /* b: 176 ~ 191 */
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  /* c: 192 ~ 207 */
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  /* d: 208 ~ 223 */
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  /* e: 224 ~ 239 */
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  /* f: 240 ~ 255 */
};

/*
 * Table has a 0 if that character doesn't need to be encoded;
 * otherwise it has a string with the character encoded in hex digits.
 */
static char* uri_encode_tbl[256] = {
/*    0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f */
   "%00","%01","%02","%03","%04","%05","%06","%07","%08","%09","%0a","%0b","%0c","%0d","%0e","%0f",  /* 0:   0 ~  15 */
   "%10","%11","%12","%13","%14","%15","%16","%17","%18","%19","%1a","%1b","%1c","%1d","%1e","%1f",  /* 1:  16 ~  31 */
   "%20","%21","%22","%23","%24","%25","%26","%27","%28","%29","%2a","%2b","%2c",   0 ,   0 ,"%2f",  /* 2:  32 ~  47 */
      0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,"%3a","%3b","%3c","%3d","%3e","%3f",  /* 3:  48 ~  63 */
   "%40",   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,  /* 4:  64 ~  79 */
      0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,"%5b","%5c","%5d","%5e",   0 ,  /* 5:  80 ~  95 */
   "%60",   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,  /* 6:  96 ~ 111 */
      0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,"%7b","%7c","%7d",   0 ,"%7f",  /* 7: 112 ~ 127 */
   "%80","%81","%82","%83","%84","%85","%86","%87","%88","%89","%8a","%8b","%8c","%8d","%8e","%8f",  /* 8: 128 ~ 143 */
   "%90","%91","%92","%93","%94","%95","%96","%97","%98","%99","%9a","%9b","%9c","%9d","%9e","%9f",  /* 9: 144 ~ 159 */
   "%a0","%a1","%a2","%a3","%a4","%a5","%a6","%a7","%a8","%a9","%aa","%ab","%ac","%ad","%ae","%af",  /* a: 160 ~ 175 */
   "%b0","%b1","%b2","%b3","%b4","%b5","%b6","%b7","%b8","%b9","%ba","%bb","%bc","%bd","%be","%bf",  /* b: 176 ~ 191 */
   "%c0","%c1","%c2","%c3","%c4","%c5","%c6","%c7","%c8","%c9","%ca","%cb","%cc","%cd","%ce","%cf",  /* c: 192 ~ 207 */
   "%d0","%d1","%d2","%d3","%d4","%d5","%d6","%d7","%d8","%d9","%da","%db","%dc","%dd","%de","%df",  /* d: 208 ~ 223 */
   "%e0","%e1","%e2","%e3","%e4","%e5","%e6","%e7","%e8","%e9","%ea","%eb","%ec","%ed","%ee","%ef",  /* e: 224 ~ 239 */
   "%f0","%f1","%f2","%f3","%f4","%f5","%f6","%f7","%f8","%f9","%fa","%fb","%fc","%fd","%fe","%ff",  /* f: 240 ~ 255 */
};

int uri_decode(Slice encoded, Buffer *decoded) {
    uint32_t orig = decoded->len;
    for (uint32_t p = 0; p < encoded.len; ) {
        if (encoded.ptr[p+0] == '%') {
            if ((p+2) < encoded.len &&
                isxdigit(encoded.ptr[p+1]) &&
                isxdigit(encoded.ptr[p+2])) {
                /* put a byte together from the next two hex digits */
                uint8_t b = MAKE_BYTE(uri_decode_tbl[(int)encoded.ptr[p+1]],
                        uri_decode_tbl[(int)encoded.ptr[p+2]]);
                buffer_append_byte(decoded, b);
                /* we used up 3 characters (%XY) from source */
                p += 3;
            } else {
                return 0;
            }
        } else {
            buffer_append_byte(decoded, encoded.ptr[p+0]);
            p += 1;
        }
    }
    return decoded->len - orig;
}

int uri_encode(Slice decoded, Buffer *encoded) {
    uint32_t orig = encoded->len;
    for (uint32_t p = 0; p < decoded.len; ++p) {
        unsigned char b = decoded.ptr[p];
        char* v = uri_encode_tbl[(int)b];

        // current source character doesn't need encoding => copy it
        if (!v) {
            buffer_append_byte(encoded, b);
            continue;
        }

        // copy encoded character from our table */
        buffer_append_string(encoded, v, 3);
    }

    return encoded->len - orig;
}
