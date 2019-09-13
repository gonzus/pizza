#include <stdio.h>
#include <string.h>
#include "pizza.h"

static unsigned char volcano[] = {
    0x45,
    0x79,
    0x6A,
    0x61,
    0x66,
    0x6A,
    0x61,
    0x6C,
    0x6C,
    0x61,
    0x6A,
    0xC3,
    0xB6,
    0x6B,
    0x75,
    0x6C,
    0x6C,
    0x00,
    '!',
};

static void test_simple(void) {
    const char* name = "nico";
    int year = 2005;
    char tmp[100];

    slice s1 = slice_wrap_ptr(name);

    buffer* b = buffer_build();
    buffer_set_slice(b, s1);

    sprintf(tmp, " was born in %d", year);
    buffer_append_slice(b, slice_wrap_ptr(tmp));
    buffer_append_character(b, '!');
    slice s2 = buffer_get_slice(b);

    slice_to_string(s2, tmp);
    printf("[%lu] [%s]\n", strlen(tmp), tmp);

    buffer_set_character(b, '(');
    buffer_append_slice(b, slice_wrap_ptr("it was Sofi who was born in 2002, before Nico"));
    buffer_append_character(b, ')');
    slice s3 = buffer_get_slice(b);

    slice_to_string(s3, tmp);
    printf("[%lu] [%s]\n", strlen(tmp), tmp);

    buffer_set_slice(b, slice_wrap_ptr("Bye now!"));
    slice s4 = buffer_get_slice(b);
    slice_to_string(s4, tmp);
    printf("[%lu] [%s]\n", strlen(tmp), tmp);
    buffer_destroy(b);
}

static void test_utf8(void) {
    buffer* b = buffer_build();
    for (unsigned int j = 0; j < sizeof(volcano); ++j) {
        buffer_append_character(b, volcano[j]);
#if 0
        buffer_append_character(b, volcano[j]);
        buffer_append_character(b, volcano[j]);
#endif
    }
    slice s1 = buffer_get_slice(b);
    char tmp[100];
    slice_to_string(s1, tmp);
    slice_dump(s1);
    buffer_destroy(b);
}

int main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    fprintf(stderr, "sizeof(slice) = %lu\n", sizeof(slice));
    fprintf(stderr, "sizeof(buffer) = %lu\n", sizeof(buffer));

    test_simple();
    test_utf8();

    return 0;
}
