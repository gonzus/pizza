#include <string.h>
#include <time.h>
#include <tap.h>
#include "slice.h"

#define ALEN(a) (int) ((sizeof(a) / sizeof((a)[0])))

static const char* empty_string = "";

static void test_slice_is_null(void) {
    cmp_ok(!!slice_is_null(SLICE_NULL), "==", !!1, "slice_is_null(SLICE_NULL)");
}

static void test_slice_is_empty(void) {
    Slice s = slice_wrap_string(empty_string);
    cmp_ok(!!slice_is_empty(s), "==", !!1, "slice_is_empty(\"\")");
}

static void test_slice_compare(void) {
    static struct {
        const char* l;
        const char* r;
        int cmp;
    } string_info[] = {
        { "foo", "bar", 1 },
        { "", "", 0 },
        { "bilbo", "frodo", -1 },
    };

    for (int j = 0; j < ALEN(string_info); ++j) {
        const char* L = string_info[j].l;
        const char* R = string_info[j].r;
        Slice l = slice_wrap_string(L);
        Slice r = slice_wrap_string(R);
        int e = string_info[j].cmp;
        int c = slice_compare(l, r);
        int ok = e == 0 ? c == 0
               : e >  0 ? c >  0
               :          c <  0;
        char label[999];
        sprintf(label, "slice_compare([%s], [%s]) => %d", L, R, e);
        cmp_ok(!!ok, "==", !!1, label);
    }
}

static void test_slice_find_byte(void) {
    static struct {
        const char* w;
        Byte b;
        int pos;
    } string_info[] = {
        { "foo", 'o', 1 },
        { "foo", 'k', -1 },
        { "", 'k', -1 },
        { "", '\0', -1 },
    };

    for (int j = 0; j < ALEN(string_info); ++j) {
        const char* W = string_info[j].w;
        Byte B = string_info[j].b;
        int e = string_info[j].pos;
        Slice w = slice_wrap_string(W);
        Slice f = slice_find_byte(w, B);
        char label[999];
        if (e < 0) {
            sprintf(label, "slice_find_byte([%s], [0x%02x]) => NOPE", W, (unsigned int) B);
            cmp_ok(!!slice_is_null(f), "==", !!1, label);
        } else {
            sprintf(label, "slice_find_byte([%s], [%c]) => OK", W, (unsigned int) B);
            cmp_ok(*f.ptr, "==", W[e], label);
        }
    }
}

static void test_slice_find_slice(void) {
    static struct {
        const char* w;
        const char* n;
        int pos;
    } string_info[] = {
        { "foo", "oo", 1 },
        { "foo", "ok", -1 },
        { "", "k", -1 },
        { "", "foo", -1 },
    };

    for (int j = 0; j < ALEN(string_info); ++j) {
        const char* W = string_info[j].w;
        const char* N = string_info[j].n;
        int e = string_info[j].pos;
        Slice w = slice_wrap_string(W);
        Slice n = slice_wrap_string(N);
        Slice f = slice_find_slice(w, n);
        char label[999];
        if (e < 0) {
            sprintf(label, "slice_find_slice([%s], [%s]) => NOPE", W, N);
            cmp_ok(!!slice_is_null(f), "==", !!1, label);
        } else {
            int len = strlen(N);
            sprintf(label, "slice_find_slice([%s], [%s]) => %d bytes", W, N, len);
            cmp_ok(f.len, "==", len, label);
            sprintf(label, "slice_find_slice([%s], [%s]) => OK", W, N);
            cmp_mem(f.ptr, N, f.len, label);
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
    };

    for (int j = 0; j < ALEN(string_info); ++j) {
        const char* STR = string_info[j].str;
        const char* SEP = string_info[j].sep;

        Slice str = slice_wrap_string(STR);
        Slice sep = slice_wrap_string(SEP);
        Slice tok = SLICE_NULL;

        char buf[1024];
        strcpy(buf, STR);
        char* carg = buf;
        int pos = 0;
        int sdone = 0;
        int cdone = 0;
        while (1) {
            if (!slice_tokenize(str, sep, &tok)) {
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
                char label[999];
                sprintf(label, "slice_tokenize([%s]) => %d tokens", STR, pos);
                ok(1, label);
                break;
            }
            if (cdone) {
                char label[999];
                sprintf(label, "slice_tokenize([%s]) token #%d sought by strtok", STR, pos);
                ok(0, label);
                break;
            }
            if (sdone) {
                char label[999];
                sprintf(label, "slice_tokenize([%s]) token #%d sought by Slice", STR, pos);
                ok(0, label);
                break;
            }
            char label[999];
            int len = strlen(ctok);
            sprintf(label, "slice_tokenize([%s]) => token %d with %d bytes", STR, pos, len);
            cmp_ok(tok.len, "==", len, label);
            sprintf(label, "slice_tokenize([%s]) => token %d = [%s]", STR, pos, ctok);
            cmp_mem(tok.ptr, ctok, tok.len, label);
            ++pos;
        }
    }
}

static void test_slice_split(void) {
    typedef size_t (*Func)(const char *s, const char *reject);
    static Func func[] = {
        strcspn,
        strspn,
    };

    static struct {
        const char* str;
        const char* set;
    } string_info[] = {
        { "", " " },
        { "X", " " },
        { "X", "X" },
        { "foo bar baz", " " },
        { "3+4*5-11+323", "+-*/" },
        { "-+3+4*5", "+-*/" },
        { "3+4*5**//", "+-*/" },
        { "-+3+4*5", "%" },
    };

    for (int j = 0; j < ALEN(string_info); ++j) {
        const char* STR = string_info[j].str;
        const char* SET = string_info[j].set;

        Slice str = slice_wrap_string(STR);
        Slice set = slice_wrap_string(SET);
        for (int k = 0; k < 2; ++k) {
            Func strfp = func[k];
            bool inc = (bool) k;
            Slice l;
            slice_split(str, inc, set, &l, 0);
            int len = strfp(STR, SET);
            char label[999];
            sprintf(label, "slice_split([%s], [%s], %s) => string with %d bytes", STR, SET, inc ? "inc" : "exc", len);
            cmp_ok(l.len, "==", len, label);
            sprintf(label, "slice_split([%s], [%s], %s) => [%*.s]", STR, SET, inc ? "inc" : "exc", len, STR);
            cmp_mem(l.ptr, STR, l.len, label);
        }
    }
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_slice_is_null();
    test_slice_is_empty();
    test_slice_compare();
    test_slice_find_byte();
    test_slice_find_slice();
    test_slice_tokenize();
    test_slice_split();

    done_testing();
}
