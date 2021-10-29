#include <string.h>
#include <tap.h>
#include "util.h"
#include "slice.h"

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
    } string_info[] = {
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

    for (int j = 0; j < ALEN(string_info); ++j) {
        const char* L = string_info[j].l;
        const char* R = string_info[j].r;
        Slice l = slice_from_string(L, 0);
        Slice r = slice_from_string(R, 0);
        int e = string_info[j].cmp;
        int c = slice_compare(l, r);
        int ok = e == 0 ? c == 0
               : e >  0 ? c >  0
               :          c <  0;
        cmp_ok(!!ok, "==", !!1, "slice_compare([%s], [%s]) => %d OK", L, R, e);
    }
}

static void test_slice_find_byte(void) {
    static struct {
        const char* w;
        char b;
        int pos;
    } string_info[] = {
        { "foo"                 , 'o'  ,  1 },
        { "foo"                 , 'k'  , -1 },
        { ""                    , 'k'  , -1 },
        { ""                    , '\0' , -1 },
        { "you know it is there", 'k'  ,  4 },
        { "this time it is not" , 'x'  , -1 },
        { ""                    , 'x'  , -1 },
    };

    for (int j = 0; j < ALEN(string_info); ++j) {
        const char* W = string_info[j].w;
        char B = string_info[j].b;
        int e = string_info[j].pos;
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
    } string_info[] = {
        { "foo"                 , "oo"    ,  1 },
        { "foo"                 , "ok"    , -1 },
        { ""                    , "k"     , -1 },
        { ""                    , "foo"   , -1 },
        { "you know it is there", "know"  ,  4 },
        { "this time it is not" , "really", -1 },
        { ""                    , "really", -1 },
        { "hello"               , ""      ,  0 },
    };

    for (int j = 0; j < ALEN(string_info); ++j) {
        const char* W = string_info[j].w;
        const char* N = string_info[j].n;
        int e = string_info[j].pos;
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
    } string_info[] = {
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

    for (int j = 0; j < ALEN(string_info); ++j) {
        const char* STR = string_info[j].str;
        const char* SEP = string_info[j].sep;

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
                ok(1, "slice_tokenize([%s]) => FOUND %d tokens", STR, pos);
                break;
            }
            if (cdone) {
                ok(0, "slice_tokenize([%s]) token #%d found by slice, NOT by strtok", STR, pos);
                break;
            }
            if (sdone) {
                ok(0, "slice_tokenize([%s]) token #%d found by strtok, NOT by slice", STR, pos);
                break;
            }
            int len = strlen(ctok);
            cmp_ok(lookup.result.len, "==", len, "slice_tokenize([%s]) => token %d with %d bytes", STR, pos, len);
            cmp_mem(lookup.result.ptr, ctok, lookup.result.len, "slice_tokenize([%s]) => token %d = [%s] OK", STR, pos, ctok);
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
    test_slice_find_byte();
    test_slice_find_slice();
    test_slice_tokenize();

    done_testing();
}
