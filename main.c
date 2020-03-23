#include <stdio.h>
#include <string.h>

#include <locale.h>     // printing Unicode
#include <wchar.h>      // wint_t

#include "slice.h"
#include "buffer.h"
#include "utf8.h"
#include "date.h"
#include "log.h"

#define ALEN(a) (sizeof(a) / sizeof(a[0]))

static void test_simple(void) {
    const char* name = "nico";
    int year = 2005;
    char tmp[100];

    Slice s1 = slice_wrap_string(name);

    sprintf(tmp, " was born in %d", year);
    Buffer* b = buffer_build();
    buffer_append_slice(b, s1);
    buffer_append_slice(b, slice_wrap_string(tmp));
    buffer_append_byte(b, '!');
    Slice s2 = buffer_get_slice(b);

    buffer_dump(b);
    printf("[%u] [%.*s]\n", s2.len, s2.len, s2.ptr);

    buffer_clear(b);
    buffer_append_byte(b, '(');
    buffer_append_slice(b, slice_wrap_string("it was Sofi who was born in 2002, before Nico"));
    buffer_append_byte(b, ')');
    Slice s3 = buffer_get_slice(b);

    printf("[%u] [%.*s]\n", s3.len, s3.len, s3.ptr);

    buffer_clear(b);
    buffer_append_slice(b, slice_wrap_string("Bye now!"));
    Slice s4 = buffer_get_slice(b);
    printf("[%u] [%.*s]\n", s4.len, s4.len, s4.ptr);
    buffer_destroy(b);
}

static void test_utf8(void) {
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

    setlocale(LC_ALL, ""); // just to print Unicode

    fprintf(stderr, "sizeof(Slice) = %lu (Byte %lu, Size %lu, Byte* %lu)\n",
            sizeof(Slice), sizeof(Byte), sizeof(Size), sizeof(Byte*));
    fprintf(stderr, "sizeof(Buffer) = %lu (wanted %lu -> %s, data %lu, fields %lu)\n",
            sizeof(Buffer), BUFFER_DESIRED_SIZE,
            sizeof(Buffer) == BUFFER_DESIRED_SIZE ? "GOOD" : "BAD",
            BUFFER_DATA_SIZE, BUFFER_FIELDS_SIZE);

    fprintf(stderr, "sizeof(Byte) = %lu\n", sizeof(Byte));
    fprintf(stderr, "sizeof(Rune) = %lu\n", sizeof(Rune));
    fprintf(stderr, "sizeof(wint_t) = %lu\n", sizeof(wint_t));

    Buffer* b = buffer_build();
    for (unsigned int j = 0; j < sizeof(volcano); ++j) {
        buffer_append_byte(b, volcano[j]);
    }
    Slice s = buffer_get_slice(b);
    slice_dump(s);

    fprintf(stderr, "-- Decoding --\n");
    int ulen = 0;
    Rune uni[1024];
    for (Slice x; !slice_is_empty(s); s = x) {
        Rune r = utf8_decode(s, &x);
        if (r == UTF8_INVALID_RUNE) {
            fprintf(stderr, "FUCK\n");
            continue;
        }
        uni[ulen++] = r;
        fprintf(stderr, "Rune [%x:%u]: [%lc]\n", r, r, (wint_t) r);
    }
    fprintf(stderr, "Decoded %d bytes into %d runes\n", buffer_length(b), ulen);

    fprintf(stderr, "-- Encoding --\n");
    Buffer *enc = buffer_build();
    for (int j = 0; j < ulen; ++j) {
        Rune r = uni[j];
        unsigned int len = utf8_encode(r, enc);
        if (len == 0) {
            fprintf(stderr, "FUCK\n");
            continue;
        }
    }
    fprintf(stderr, "Encoded %d runes into %d bytes\n", ulen, buffer_length(enc));
    s = buffer_get_slice(enc);
    slice_dump(s);

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
    for (unsigned int j = 0; j < ALEN(data); ++j) {
        fprintf(stderr, "Tokenizing [%s], separators [%s]\n", data[j].string, data[j].sep);
        Slice s = slice_wrap_string(data[j].string);
        Slice sep = slice_wrap_string(data[j].sep);
        for (Slice token = SLICE_NULL; slice_tokenize(s, sep, &token); ) {
            fprintf(stderr, "  [%.*s]\n", token.len, token.ptr);
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
    for (unsigned int j = 0; j < ALEN(data); ++j) {
        fprintf(stderr, "Splitting [%s], set [%s]\n", data[j].string, data[j].set);
        Slice s = slice_wrap_string(data[j].string);
        Slice set = slice_wrap_string(data[j].set);
        for (int included = 1; 1; included = !included) {
            Slice l, r;

            if (slice_get_length(s) == 0) break;

            slice_split(s, included, set, &l, &r);
            fprintf(stderr, "  L (%d) [%.*s]\n", included, l.len, l.ptr);
            fprintf(stderr, "  R (%d) [%.*s]\n", included, r.len, r.ptr);
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
    for (unsigned int j = 0; j < ALEN(data); ++j) {
        Slice l = slice_wrap_string(data[j].l);
        Slice r = slice_wrap_string(data[j].r);
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
    for (unsigned int j = 0; j < ALEN(data); ++j) {
        Slice s = slice_wrap_string(data[j].s);
        Slice f = slice_find_byte(s, data[j].t);
        fprintf(stderr, "Searching byte [%c] in slice [%s] => %s - [%.*s]\n",
                data[j].t, data[j].s, slice_is_null(f) ? "N" : "Y", f.len, f.ptr);
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
    for (unsigned int j = 0; j < ALEN(data); ++j) {
        Slice s = slice_wrap_string(data[j].s);
        Slice t = slice_wrap_string(data[j].t);
        Slice f = slice_find_slice(s, t);
        fprintf(stderr, "Searching slice [%s] in [%s] => %s - [%.*s]\n",
                data[j].t, data[j].s, slice_is_null(f) ? "N" : "Y", f.len, f.ptr);
    }
}

static void test_clone(void) {
    static struct {
        const char* s;
    } data[] = {
        { "you know it is there" },
        { "" },
        { "  this looks \t\t good " },
    };
    for (unsigned int j = 0; j < ALEN(data); ++j) {
        Slice s = slice_wrap_string(data[j].s);
        Buffer* b = buffer_build();
        buffer_append_slice(b, s);
        Buffer* n = buffer_clone(b);
        fprintf(stderr, "Cloning [%s]: [%d:%d:%p] [%d:%d:%p] %s\n",
                data[j].s, b->pos, b->cap, b->ptr, n->pos, n->cap, n->ptr,
                b->pos == n->pos && memcmp(b->ptr, n->ptr, b->pos) == 0 ? "OK" : "BAD");
        buffer_destroy(b);
    }
}

static void test_pack(void) {
    static struct {
        const char* s;
    } data[] = {
        { "you know it is there" },
        { "" },
        { "  this looks \t\t good, hopefully it is " },
    };
    for (unsigned int j = 0; j < ALEN(data); ++j) {
        Slice s = slice_wrap_string(data[j].s);
        Buffer* b = buffer_build();
        buffer_append_slice(b, s);
        buffer_pack(b);
        fprintf(stderr, "Packing [%s]: [%d:%d:%p] [%.*s]\n",
                data[j].s, b->pos, b->cap, b->ptr, b->pos, b->ptr);
        buffer_destroy(b);
    }
    Buffer* b = buffer_build();
    const char* heap = "extremely useful heap";
    buffer_format_print(b, "This is large enough to need the %s %s %s", heap, heap, heap);
    buffer_clear(b);
    buffer_pack(b);
    buffer_destroy(b);
}

static void test_format_numbers(void) {
    static struct {
        long long int u;
        unsigned long long int s;
        double d;
        int t;
    } data[] = {
        { 0ULL , 0, 0, 0 },
        { 1ULL , 0, 0, 0 },
        { 11ULL, 0, 0, 0 },

        { 0,   0LL, 0, 1 },
        { 0,   1LL, 0, 1 },
        { 0,  -1LL, 0, 1 },
        { 0,  12LL, 0, 1 },
        { 0, -12LL, 0, 1 },

        { 0, 0,   0.0, 2 },
        { 0, 0,   1.0, 2 },
        { 0, 0,  -1.0, 2 },
        { 0, 0,  3.14, 2 },
        { 0, 0, -3.14, 2 },
    };
    for (unsigned int j = 0; j < ALEN(data); ++j) {
        char tmp[1024];
        Buffer* b = buffer_build();
        switch (data[j].t) {
            case 0:
                buffer_format_unsigned(b, data[j].u);
                sprintf(tmp, "%llu", data[j].u);
                break;
            case 1:
                buffer_format_signed(b, data[j].s);
                sprintf(tmp, "%lld", data[j].s);
                break;
            case 2:
                buffer_format_double(b, data[j].d);
                sprintf(tmp, "%f", data[j].d);
                break;
            default:
                continue;
        }

        Slice r = buffer_get_slice(b);
        Slice e = slice_wrap_string(tmp);
        fprintf(stderr, "Format [%s]: [%d:%.*s] %s\n",
                tmp, r.len, r.len, r.ptr, slice_compare(r, e) == 0 ? "OK" : "BAD");
        buffer_destroy(b);
    }
}

static void test_format_printf(void) {
    Buffer* b = buffer_build();

    buffer_clear(b);
    buffer_format_print(b, " Movie year %d, rating %.1f, name [%5.5s]", 1968, 8.3, "2001");
    Slice r = buffer_get_slice(b);
    Slice e = slice_wrap_string(" Movie year 1968, rating 8.3, name [ 2001]");
    fprintf(stderr, "Format [%d:%.*s] %s\n", r.len, r.len, r.ptr, slice_compare(r, e) == 0 ? "OK" : "BAD");
    buffer_destroy(b);
}

static void test_stack(void) {
    // Buffer* b = buffer_build();
    Buffer b;
    buffer_init(&b);

    buffer_clear(&b);
    buffer_format_print(&b, "This fits on the %s", "stack");
    buffer_destroy(&b);
}

static void test_stack_heap(void) {
    // Buffer* b = buffer_build();
    Buffer b;
    buffer_init(&b);

    const char* heap = "extremely useful heap";
    buffer_format_print(&b, "This is large enough to need the %s %s %s", heap, heap, heap);
    buffer_destroy(&b);
}

static void test_pure_heap(void) {
    Buffer* b = buffer_build();

    const char* heap = "extremely useful heap";
    buffer_format_print(b, "This is large enough to need the %s %s %s", heap, heap, heap);
    buffer_destroy(b);
}

static void test_dates(void) {
    int y = 0;
    int m = 0;
    int d = 0;
    int t = 0;

    t = date_today(&y, &m, &d);
    fprintf(stderr, "Today is %d %d (%s) %d = %d\n", y, m, date_month_name(m), d, t);
    fprintf(stderr, "Current year %d %s a leap year\n", y, date_is_leap_year(y) ? "IS" : "IS NOT");
    fprintf(stderr, "Current month %d %d (%s) has %d days\n", y, m, date_month_name(m), date_days_in_month(y, m));
    int j2 = date_ymd_to_julian(y, m, d);

    t = date_easter(y, &m, &d);
    fprintf(stderr, "Easter for year %d is on %d (%s) %d = %d\n", y, m, date_month_name(m), d, t);
    int j1 = date_ymd_to_julian(y, m, d);
    fprintf(stderr, "Between Easter %d and today, %d days have passed\n", y, j2 - j1);
}

int main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_utf8();
#if 1
    test_simple();
    test_tokenize();
    test_split();
    test_compare();
    test_find_byte();
    test_find_slice();
    test_clone();
    test_pack();
    test_format_numbers();
    test_format_printf();
    test_stack();
    test_stack_heap();
    test_pure_heap();
    test_dates();
#endif

    return 0;
}
