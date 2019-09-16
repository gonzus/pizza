#include <stdio.h>
#include <string.h>
#include <slice.h>
#include <buffer.h>

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
    unsigned int len = 0;

    slice s1 = slice_wrap_string(name);

    buffer* b = buffer_build();
    buffer_set_slice(b, s1);

    sprintf(tmp, " was born in %d", year);
    buffer_append_slice(b, slice_wrap_string(tmp));
    buffer_append_byte(b, '!');
    slice s2 = buffer_get_slice(b);

    len = slice_to_string(s2, tmp);
    printf("[%u] [%s]\n", len, tmp);

    buffer_set_byte(b, '(');
    buffer_append_slice(b, slice_wrap_string("it was Sofi who was born in 2002, before Nico"));
    buffer_append_byte(b, ')');
    slice s3 = buffer_get_slice(b);

    len = slice_to_string(s3, tmp);
    printf("[%u] [%s]\n", len, tmp);

    buffer_set_slice(b, slice_wrap_string("Bye now!"));
    slice s4 = buffer_get_slice(b);
    len = slice_to_string(s4, tmp);
    printf("[%u] [%s]\n", len, tmp);
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

static void test_split(void) {
    static struct {
        const char* string;
        const char* set;
    } data[] = {
        { " this is  a   line with\tblanks ", " \t" },
        { "123+34*-55-+/28+++--", "+-*/" },
        { "--++123", "+-*/" },
    };
    char tmp[1024];
    for (unsigned int j = 0; j < sizeof(data) / sizeof(data[0]); ++j) {
        fprintf(stderr, "Splitting [%s], set [%s]\n", data[j].string, data[j].set);
        slice s = slice_wrap_string(data[j].string);
        slice set = slice_wrap_string(data[j].set);
        for (int included = 1; 1; included = !included) {
            slice l, r;

            if (slice_get_length(s) == 0) break;

            slice_split(s, included, set, &l, &r);
            slice_to_string(l, tmp);
            fprintf(stderr, "  L [%s]\n", tmp);
            slice_to_string(r, tmp);
            fprintf(stderr, "  R [%s]\n", tmp);
            s = r;
        }

        fprintf(stderr, "Splitting DONE\n");
    }
}

static void test_compare(void) {
    static struct {
        const char* l;
        const char* r;
    } data[] = {
        { "abc" , "pqr"  },
        { "pqr" , "abc"  },
        { "pqr" , "pqr"  },
        { ""    , ""     },
        { "abc" , "abcd" },
        { "abcd", "abc"  },
        { "abcd", "abcd" },
    };
    for (unsigned int j = 0; j < sizeof(data) / sizeof(data[0]); ++j) {
        slice l = slice_wrap_string(data[j].l);
        slice r = slice_wrap_string(data[j].r);
        fprintf(stderr, "Comparing [%s] [%s] => %d\n", data[j].l, data[j].r, slice_compare(l, r));
    }
}

static void test_find_byte(void) {
    static struct {
        const char* s;
        char t;
    } data[] = {
        { "you know it is there" , 'k'  },
        { "this time it is not" , 'x'  },
        { "" , 'x'  },
    };
    char tmp[1024];
    for (unsigned int j = 0; j < sizeof(data) / sizeof(data[0]); ++j) {
        slice s = slice_wrap_string(data[j].s);
        slice f = slice_find_byte(s, data[j].t);
        slice_to_string(f, tmp);
        fprintf(stderr, "Searching byte [%c] in slice [%s] => %d - [%s]\n", data[j].t, data[j].s, !slice_is_null(f), tmp);
    }
}

static void test_find_slice(void) {
    static struct {
        const char* s;
        const char* t;
    } data[] = {
        { "you know it is there" , "know"  },
        { "this time it is not" , "really"  },
        { "" , "really"  },
        { "hello" , ""  },
    };
    char tmp[1024];
    for (unsigned int j = 0; j < sizeof(data) / sizeof(data[0]); ++j) {
        slice s = slice_wrap_string(data[j].s);
        slice t = slice_wrap_string(data[j].t);
        slice f = slice_find_slice(s, t);
        slice_to_string(f, tmp);
        fprintf(stderr, "Searching slice [%s] in [%s] => %s - [%s]\n",
                data[j].t, data[j].s, slice_is_null(f) ? "N" : "Y", tmp);
    }
}

static void test_clone(void) {
    static struct {
        const char* s;
    } data[] = {
        { "you know it is there" },
        { "" },
        { "  this look \t\t good " },
    };
    for (unsigned int j = 0; j < sizeof(data) / sizeof(data[0]); ++j) {
        slice s = slice_wrap_string(data[j].s);
        buffer* b = buffer_build();
        buffer_append_slice(b, s);
        buffer* n = buffer_clone(b);
        fprintf(stderr, "Cloning [%s]: [%d:%d:%p] [%d:%d:%p] %s\n",
                data[j].s, b->pos, b->cap, b->ptr, n->pos, n->cap, n->ptr,
                b->pos == n->pos && memcmp(b->ptr, n->ptr, b->pos) == 0 ? "OK" : "BAD");
    }
}

static void test_pack(void) {
    static struct {
        const char* s;
    } data[] = {
        { "you know it is there" },
        { "" },
        { "  this look \t\t good, hopefully it is " },
    };
    for (unsigned int j = 0; j < sizeof(data) / sizeof(data[0]); ++j) {
        slice s = slice_wrap_string(data[j].s);
        buffer* b = buffer_build();
        buffer_append_slice(b, s);
        buffer_pack(b);
        fprintf(stderr, "Packing [%s]: [%d:%d:%p] %s\n",
                data[j].s, b->pos, b->cap, b->ptr,
                b->pos == b->cap && (b->ptr ? b->pos == strlen((char*) b->ptr) : b->pos == 0) ? "OK" : "BAD");
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
    test_split();
    test_compare();
    test_find_byte();
    test_find_slice();
    test_clone();
    test_pack();

    return 0;
}
