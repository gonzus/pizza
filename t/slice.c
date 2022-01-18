#include <string.h>
#include <tap.h>
#include "pizza/util.h"
#include "pizza/slice.h"

#define ALEN(a) (int) ((sizeof(a) / sizeof((a)[0])))

static const char* empty_string = "";

static void test_sizes(void) {
    int size_ptr = sizeof(void*);
    cmp_ok(sizeof(Slice), "==", 2*size_ptr, "sizeof(Slice)");
    cmp_ok(sizeof(char) , "==", 1         , "sizeof(char)");
    cmp_ok(sizeof(char*), "==", size_ptr  , "sizeof(char*)");
}

static void test_slice_is_empty(void) {
    Slice s = slice_from_string(empty_string, 0);
    cmp_ok(!!slice_is_empty(s), "==", !!1, "slice_is_empty(\"\")");
}

static void test_slice_compare(void) {
    static struct {
        const char* l;
        const char* r;
        int cmp;
    } data[] = {
        { "foo"  , "bar"  ,  1 },
        { ""     , ""     ,  0 },
        { "bilbo", "frodo", -1 },
        { "abc"  , "pqr"  , -1 },
        { "pqr"  , "abc"  ,  1 },
        { "pqr"  , "pqr"  ,  0 },
        { ""     , ""     ,  0 },
        { "abc"  , "abcd" , -1 },
        { "abcd" , "abc"  ,  1 },
        { "abcd" , "abcd" ,  0 },
    };

    for (int j = 0; j < ALEN(data); ++j) {
        const char* L = data[j].l;
        const char* R = data[j].r;
        Slice l = slice_from_string(L, 0);
        Slice r = slice_from_string(R, 0);
        int e = data[j].cmp;
        int c = slice_compare(l, r);
        int ok = e == 0 ? c == 0
               : e >  0 ? c >  0
               :          c <  0;
        cmp_ok(!!ok, "==", !!1, "slice_compare([%s], [%s]) => %d OK", L, R, e);
    }
}

static void test_slice_trim(void) {
    static struct {
        const char* label;
        const char* s;
        const char* t;
    } data[] = {
        { "empty"                    , ""                                   , ""      },
        { "one space"                , " "                                  , ""      },
        { "one tab"                  , "\t"                                 , ""      },
        { "one newline"              , "\n"                                 , ""      },
        { "only before"              , " bilbo"                             , "bilbo" },
        { "only after"               , "bilbo "                             , "bilbo" },
        { "both before and after"    , " bilbo "                            , "bilbo" },
        { "spaces, tabs and newlines", " \t \t\t\n  \n bilbo \t\n\n\t\t  \t", "bilbo" },
    };

    for (int j = 0; j < ALEN(data); ++j) {
        const char* L = data[j].label;
        const char* S = data[j].s;
        const char* T = data[j].t;
        Slice s = slice_from_string(S, 0);
        Slice g = slice_trim(s);
        Slice e = slice_from_string(T, 0);
        ok(slice_equal(g, e), "slice_trim() for %s => [%s] OK", L, T);
    }
}

static void test_slice_int(void) {
    static struct {
        const char* label;
        const char* s;
        int r;
        int v;
    } data[] = {
        { "empty"    , ""   , 1,  0 }, // an empty slice is a valid int == 0
        { "invalid 1", "x"  , 0,  0 },
        { "invalid 3", "xyz", 0,  0 },
        { "zero"     , "0"  , 1,  0 },
        { "nine"     , "9"  , 1,  9 },
        { "sixty six", "66" , 1, 66 },
    };

    for (int j = 0; j < ALEN(data); ++j) {
        const char* L = data[j].label;
        const char* S = data[j].s;
        Slice s = slice_from_string(S, 0);
        int val = 0;
        int ret = slice_int(s, &val);
        ok(ret == data[j].r, "slice_int() returns %d = %d for %s OK", ret, data[j].r, L);
        ok(val == data[j].v, "slice_int() produces %d = %d for %s OK", val, data[j].v, L);
    }
}

static void test_slice_find_byte(void) {
    static struct {
        const char* w;
        char b;
        int pos;
    } data[] = {
        { "foo"                 , 'o'  ,  1 },
        { "foo"                 , 'k'  , -1 },
        { ""                    , 'k'  , -1 },
        { ""                    , '\0' , -1 },
        { "you know it is there", 'k'  ,  4 },
        { "this time it is not" , 'x'  , -1 },
        { ""                    , 'x'  , -1 },
    };

    for (int j = 0; j < ALEN(data); ++j) {
        const char* W = data[j].w;
        char B = data[j].b;
        int e = data[j].pos;
        Slice w = slice_from_string(W, 0);
#if 0
        dump_bytes(stderr, w.ptr, w.len);
#endif
        Slice f = slice_find_byte(w, B);
        if (e < 0) {
            cmp_ok(!!slice_is_empty(f), "==", !!1, "slice_find_byte([%s], [0x%02x]) => ABSENT", W, (unsigned int) B);
        } else {
            cmp_ok(*f.ptr, "==", W[e], "slice_find_byte([%s], [%c]) => FOUND", W, (unsigned int) B);
        }
    }
}

static void test_slice_find_slice(void) {
    static struct {
        const char* w;
        const char* n;
        int pos;
    } data[] = {
        { "foo"                 , "oo"    ,  1 },
        { "foo"                 , "ok"    , -1 },
        { ""                    , "k"     , -1 },
        { ""                    , "foo"   , -1 },
        { "you know it is there", "know"  ,  4 },
        { "this time it is not" , "really", -1 },
        { ""                    , "really", -1 },
        { "hello"               , ""      ,  0 },
    };

    for (int j = 0; j < ALEN(data); ++j) {
        const char* W = data[j].w;
        const char* N = data[j].n;
        int e = data[j].pos;
        Slice w = slice_from_string(W, 0);
        Slice n = slice_from_string(N, 0);
        Slice f = slice_find_slice(w, n);
        if (e < 0) {
            cmp_ok(!!slice_is_empty(f), "==", !!1, "slice_find_slice([%s], [%s]) => ABSENT", W, N);
        } else {
            int len = strlen(N);
            cmp_ok(f.len, "==", len, "slice_find_slice([%s], [%s]) => has %d bytes", W, N, len);
            cmp_mem(f.ptr, N, f.len, "slice_find_slice([%s], [%s]) => FOUND", W, N);
        }
    }
}

static void test_slice_tokenize(void) {
    static struct {
        const char* str;
        const char* sep;
    } data[] = {
        { "", " " },
        { "X", " " },
        { "X", "X" },
        { "foo bar baz", " " },
        { "3+4*5-11+323", "+-*/" },
        { "-+3+4*5", "+-*/" },
        { "3+4*5**//", "+-*/" },
        { "-+3+4*5", "%" },
        { "simple-line-with-single-separator", "-" },
        { "simple-line;-this-time,-with-multiple-separators", ",;" },
        { "duplicated,;-separators;;;now", ",;" },
        { "separators not found", ",;" },
        { "", ",;" },
        { "empty separators", "" },
    };

    for (int j = 0; j < ALEN(data); ++j) {
        const char* STR = data[j].str;
        const char* SEP = data[j].sep;

        Slice str = slice_from_string(STR, 0);
        Slice sep = slice_from_string(SEP, 0);
        SliceLookup lookup = {0};

        char buf[1024];
        strcpy(buf, STR);
        char* carg = buf;
        int pos = 0;
        int sdone = 0;
        int cdone = 0;
        while (1) {
            if (!slice_tokenize_by_slice(str, sep, &lookup)) {
                sdone = 1;
            }
            // if (pos > 0) sdone = 1;

            char* ctok = strtok(carg, SEP);
            carg = 0;
            if (!ctok) {
                cdone = 1;
            }
            // if (pos > 0) cdone = 1;

            if (cdone && sdone) {
                ok(1, "slice_tokenize_by_slice([%s]) => FOUND %d tokens", STR, pos);
                break;
            }
            if (cdone) {
                ok(0, "slice_tokenize_by_slice([%s]) token #%d found by slice, NOT by strtok", STR, pos);
                break;
            }
            if (sdone) {
                ok(0, "slice_tokenize_by_slice([%s]) token #%d found by strtok, NOT by slice", STR, pos);
                break;
            }
            int len = strlen(ctok);
            cmp_ok(lookup.result.len, "==", len, "slice_tokenize_by_slice([%s]) => token %d with %d bytes", STR, pos, len);
            cmp_mem(lookup.result.ptr, ctok, lookup.result.len, "slice_tokenize_by_slice([%s]) => token %d = [%s] OK", STR, pos, ctok);
            ++pos;
        }
    }
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_sizes();
    test_slice_is_empty();
    test_slice_compare();
    test_slice_trim();
    test_slice_int();
    test_slice_find_byte();
    test_slice_find_slice();
    test_slice_tokenize();

    done_testing();
}
