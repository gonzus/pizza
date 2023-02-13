#include <stdint.h>
#include <string.h>
#include <tap.h>
#include "hash.h"

#define ALEN(a) (int) ((sizeof(a) / sizeof((a)[0])))

static void test_djb2(void) {
    struct {
        const char* str;
        uint32_t hash;
    } values[] = {
        { "c++isfun", 858489443 },
        { "oopiscool", 2405320828 },
    };

    for (int j = 0; j < ALEN(values); ++j) {
        const char* str = values[j].str;
        uint32_t len = strlen(str);
        uint32_t got = hash_djb2(str, len);
        cmp_ok(got , "==", values[j].hash,
               "djb2 hash for [%.*s] is %u, looks good", len, str, got);
    }
}

static void test_murmur3(void) {
    struct {
        const char* str;
        uint32_t seed;
        uint32_t hash;
    } values[] = {
        { "kinkajou", 0, 0xb6d99cf8 },
        { "Hello, world!", 1234, 0xfaf6cdb3 },
        { "Hello, world!", 4321, 0xbf505788 },
        { "xxxxxxxxxxxxxxxxxxxxxxxxxxxx", 1234, 0x8905ac28 },
        { "", 1234, 0x0f2cc00b },
    };

    for (int j = 0; j < ALEN(values); ++j) {
        const char* str = values[j].str;
        uint32_t seed = values[j].seed;
        uint32_t len = strlen(str);
        uint32_t got = hash_murmur3(str, len, seed);
        uint32_t expected = values[j].hash;
        cmp_ok(got , "==", expected,
               "murmur3 hash for [%.*s] with seed %u is 0x%x, looks good", len, str, seed, got);
    }
}

static void test_wang(void) {
    ok(1, "DON'T HAVE ANY DATA TO COMPARE WITH FOR hash_wang()");
}

static void test_pcg(void) {
    ok(1, "DON'T HAVE ANY DATA TO COMPARE WITH FOR hash_pcg()");
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_djb2();
    test_murmur3();
    test_wang();
    test_pcg();

    done_testing();
}
