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

    slice s1 = slice_wrap_string(name);

    buffer* b = buffer_build();
    buffer_set_slice(b, s1);

    sprintf(tmp, " was born in %d", year);
    buffer_append_slice(b, slice_wrap_string(tmp));
    buffer_append_byte(b, '!');
    slice s2 = buffer_get_slice(b);

    slice_to_string(s2, tmp);
    printf("[%lu] [%s]\n", strlen(tmp), tmp);

    buffer_set_byte(b, '(');
    buffer_append_slice(b, slice_wrap_string("it was Sofi who was born in 2002, before Nico"));
    buffer_append_byte(b, ')');
    slice s3 = buffer_get_slice(b);

    slice_to_string(s3, tmp);
    printf("[%lu] [%s]\n", strlen(tmp), tmp);

    buffer_set_slice(b, slice_wrap_string("Bye now!"));
    slice s4 = buffer_get_slice(b);
    slice_to_string(s4, tmp);
    printf("[%lu] [%s]\n", strlen(tmp), tmp);
    buffer_destroy(b);
}

static void test_utf8(void) {
    buffer* b = buffer_build();
    for (unsigned int j = 0; j < sizeof(volcano); ++j) {
        buffer_append_byte(b, volcano[j]);
#if 0
        buffer_append_byte(b, volcano[j]);
        buffer_append_byte(b, volcano[j]);
#endif
    }
    slice s1 = buffer_get_slice(b);
    char tmp[100];
    slice_to_string(s1, tmp);
    slice_dump(s1);
    buffer_destroy(b);
}

static void test_tokenize(void) {
    static struct {
        const char* string;
        const char* sep;
    } data[] = {
        { "simple-line-with-single-separator", "-" },
        { "simple-line;-this-time,-with-multiple-separators", ",;" },
        { "duplicated,;-separators;;;now", ",;" },
        { "separators not found", ",;" },
        { "", ",;" },
        { "empty separators", "" },
    };
    char tmp[1024];
    for (unsigned int j = 0; j < sizeof(data) / sizeof(data[0]); ++j) {
        fprintf(stderr, "Tokenizing [%s], separators [%s]\n", data[j].string, data[j].sep);
        slice s = slice_wrap_string(data[j].string);
        slice sep = slice_wrap_string(data[j].sep);
        for (slice token = SLICE_NULL; slice_tokenize(s, sep, &token); ) {
            slice_to_string(token, tmp);
            fprintf(stderr, "  [%s]\n", tmp);
        }
        fprintf(stderr, "Tokenizing DONE\n");
    }
}

int main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    fprintf(stderr, "sizeof(slice) = %lu\n", sizeof(slice));
    fprintf(stderr, "sizeof(buffer) = %lu\n", sizeof(buffer));

    test_simple();
    test_utf8();
    test_tokenize();

    return 0;
}
