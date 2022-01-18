#include <string.h>
#include <tap.h>
#include "pizza/memory.h"
#include "pizza/deflator.h"

#define TEXT_LEN 1024

static void test_deflator(void) {
    char text[TEXT_LEN];
    for (uint32_t j = 0; j < TEXT_LEN; ++j) {
        text[j] = 'a' + j % 26;
    }
    Slice s = slice_from_memory(text, TEXT_LEN);

    Buffer c; buffer_build(&c);
    Buffer u; buffer_build(&u);
    for (int size = 1024; size <= 16384; size += 1024) {
        for (int level = 1; level <= 9; ++level) {
            buffer_clear(&c);
            buffer_clear(&u);

            Deflator deflator;
            deflator_build(&deflator, size);
            deflator_compress(&deflator, s, &c, level);
            Slice t = buffer_slice(&c);
            deflator_uncompress(&deflator, t, &u);
            deflator_destroy(&deflator);

            Slice f = buffer_slice(&u);
            ok(s.len == f.len, "Got same length %u == %u with chunk %d and level %d", s.len, f.len, size, level);
            ok(slice_equal(s, f), "Could rountrip %u bytes with chunk %d and level %d", s.len, size, level);
        }
    }
    buffer_destroy(&u);
    buffer_destroy(&c);
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_deflator();

    done_testing();
}
