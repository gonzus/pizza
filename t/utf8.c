#include <string.h>
#include <wchar.h>
#include <tap.h>
#include "pizza/utf8.h"

#define ALEN(a) (int) (sizeof(a) / sizeof(a[0]))

static uint8_t utf8[] = {
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

static void test_decode_valid(void) {
    int len = ALEN(utf8);
    Slice encoded = slice_from_memory((const char*) utf8, len);

    for (int pos = 0; !slice_is_empty(encoded); ++pos) {
        uint8_t b = encoded.ptr[0];
        uint32_t r = utf8_decode(&encoded);
        cmp_ok(r, "!=", UTF8_INVALID_RUNE, "utf8_decode(0x%x) pos %d OK", (int) b, pos);
        cmp_ok(r, "==", unicode[pos], "utf8_decode(0x%x) pos %d => [0x%x]", (int) b, pos, (unsigned int) r);
    }
}

static void test_decode_invalid(void) {
    static struct {
        int valid;
        const char* label;
        const char* encoded;
    } data[] = {
        { 1, "Valid ASCII", "a" },
        { 1, "Valid 2 Octet Sequence", "\xc3\xb1" },
        { 0, "Invalid 2 Octet Sequence", "\xc3\x28" },
        { 0, "Invalid Sequence Identifier", "\xa0\xa1" },
        { 1, "Valid 3 Octet Sequence", "\xe2\x82\xa1" },
        { 0, "Invalid 3 Octet Sequence (in 2nd Octet)", "\xe2\x28\xa1" },
        { 0, "Invalid 3 Octet Sequence (in 3rd Octet)", "\xe2\x82\x28" },
        { 1, "Valid 4 Octet Sequence", "\xf0\x90\x8c\xbc" },
        { 0, "Invalid 4 Octet Sequence (in 2nd Octet)", "\xf0\x28\x8c\xbc" },
        { 0, "Invalid 4 Octet Sequence (in 3rd Octet)", "\xf0\x90\x28\xbc" },
        { 0, "Invalid 4 Octet Sequence (in 4th Octet)", "\xf0\x28\x8c\x28" },
        { 0, "Valid 5 Octet Sequence (but not Unicode!)", "\xf8\xa1\xa1\xa1\xa1" },
        { 0, "Valid 6 Octet Sequence (but not Unicode!)", "\xfc\xa1\xa1\xa1\xa1\xa1" },

    };

    for (int j = 0; j < ALEN(data); ++j) {
        Slice encoded = slice_from_string(data[j].encoded, 0);
        uint32_t r = utf8_decode(&encoded);
        int good = data[j].valid ? (r != UTF8_INVALID_RUNE) : (r == UTF8_INVALID_RUNE);
        ok(good, "utf8_decode() returns %s for '%s'", data[j].valid ? "valid" : "invalid", data[j].label);
    }
}

static void test_encode_valid(void) {
    int ulen = sizeof(unicode) / sizeof(unicode[0]);
    Buffer encoded; buffer_build(&encoded);
    for (int j = 0; j < ulen; ++j) {
        uint32_t r = unicode[j];
        unsigned int len = utf8_encode(r, &encoded);
        cmp_ok(len, ">", 0, "utf8_encode(0x%x) => %u bytes", (int) r, len);
    }
    for (unsigned int j = 0; j < encoded.len; ++j) {
        uint8_t b = encoded.ptr[j];
        cmp_ok(b, "==", utf8[j], "utf8_encode pos %d OK => 0x%x", j, b);
    }
    buffer_destroy(&encoded);
}

static void test_encode_invalid(void) {
    static struct {
        int valid;
        uint32_t rune;
        const char* encoded;
    } data[] = {
        { 1, 0x00, "\x00" },
        { 1, 0x01, "\x01" },
        { 1, 0x7e, "\x7e" },
        { 1, 0x7f, "\x7f" },
        { 1, 0x80, "\xc2\x80" },
        { 1, 0x81, "\xc2\x81" },
        { 1, 0x07fe, "\xdf\xbe" },
        { 1, 0x07ff, "\xdf\xbf" },
        { 1, 0x0800, "\xe0\xa0\x80" },
        { 1, 0x0801, "\xe0\xa0\x81" },
        { 1, 0xfffe, "\xef\xbf\xbe" },
        { 1, 0xffff, "\xef\xbf\xbf" },
        { 1, 0x10000, "\xf0\x90\x80\x80" },
        { 1, 0x10001, "\xf0\x90\x80\x81" },
        { 1, 0x10fffe, "\xf4\x8f\xbf\xbe" },
        { 1, 0x10ffff, "\xf4\x8f\xbf\xbf" },
        { 0, 0x110000, "\xef\xbf\xbd" },
        { 0, 0x110001, "\xef\xbf\xbd" },
        { 0, 0xffffff, "\xef\xbf\xbd" },
        { 0, 0xfffffff, "\xef\xbf\xbd" },
        { 0, 0xffffffff, "\xef\xbf\xbd" },
    };

    Buffer b; buffer_build(&b);
    for (int j = 0; j < ALEN(data); ++j) {
        buffer_clear(&b);
        unsigned int len = utf8_encode(data[j].rune, &b);
        ok(b.len == len,
           "utf8_encode() returns correct length for 0x%08x", data[j].rune);
        ok(memcmp(b.ptr, data[j].encoded, b.len) == 0,
           "utf8_encode() returns correct data for 0x%08x", data[j].rune);
    }
    buffer_destroy(&b);
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_sizes();
    test_decode_valid();
    test_decode_invalid();
    test_encode_valid();
    test_encode_invalid();

    done_testing();
}
