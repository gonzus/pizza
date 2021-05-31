#include <wchar.h>
#include <tap.h>
#include "utf8.h"

static unsigned char utf8[] = {
    0x45, // E
    0x79, // y
    0x6A, // j
    0x61, // a
    0x66, // f
    0xF0, 0x9F, 0x92, 0xA9, // poop emoji
    0x6A, // j
    0x61, // a
    0x6C, // l
    0x6C, // l
    0x61, // a
    0x6A, // j
    0xC3, 0xB6, // ö (OE)
    0x6B, // k
    0x75, // u
    0x6C, // l
    0x6C, // l
    0x00, // \0
    0x21, // !
};

static uint32_t unicode[] = {
    0x45, // E
    0x79, // y
    0x6A, // j
    0x61, // a
    0x66, // f
    0x0001F4A9, // poop emoji
    0x6A, // j
    0x61, // a
    0x6C, // l
    0x6C, // l
    0x61, // a
    0x6A, // j
    0xF6, // ö (OE)
    0x6B, // k
    0x75, // u
    0x6C, // l
    0x6C, // l
    0x00, // \0
    0x21, // !
};

static void test_sizes(void) {
    int sru = sizeof(uint32_t);
    int s32 = sizeof(uint32_t);
    int swt = sizeof(wint_t);
    cmp_ok(sru , "==", s32, "sizeof(uint32_t) == %d", s32);
    cmp_ok(swt , "==", s32, "sizeof(wint_t) == %d", s32);
}

static void test_decode(void) {
    int len = sizeof(utf8) / sizeof(utf8[0]);
    Slice encoded = slice_wrap_ptr_len(utf8, len);

    int pos = 0;
    for (Slice left = encoded; !slice_is_empty(left); ) {
        uint8_t b = left.ptr[0];
        uint32_t r = utf8_decode(&left);
        cmp_ok(r, "!=", UTF8_INVALID_RUNE, "utf8_decode(0x%x) pos %d OK", (int) b, pos);
        cmp_ok(r, "==", unicode[pos], "utf8_decode(0x%x) pos %d => [0x%x]", (int) b, pos, (unsigned int) r);
        ++pos;
    }
}

static void test_encode(void) {
    int ulen = sizeof(unicode) / sizeof(unicode[0]);
    Buffer *encoded = buffer_create();
    for (int j = 0; j < ulen; ++j) {
        uint32_t r = unicode[j];
        unsigned int len = utf8_encode(r, encoded);
        cmp_ok(len, ">", 0, "utf8_encode(0x%x) => %u bytes", (int) r, len);
    }
    for (unsigned int j = 0; j < encoded->len; ++j) {
        uint8_t b = encoded->ptr[j];
        cmp_ok(b, "==", utf8[j], "utf8_encode pos %d OK => 0x%x", j, b);
    }
    buffer_destroy(encoded);
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_sizes();
    test_decode();
    test_encode();

    done_testing();
}
