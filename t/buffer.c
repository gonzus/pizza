#include <tap.h>
#include "buffer.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define ALEN(a) (sizeof(a) / sizeof(a[0]))

static void test_sizes(void) {
    int sb = sizeof(Buffer);
    int bde = BUFFER_DESIRED_SIZE;
    int bda = BUFFER_DATA_SIZE;
    int bfi = BUFFER_FIELDS_SIZE;

    cmp_ok(sb, "==", bde, "sizeof(Buffer) = %d", bde);
    cmp_ok(sb, "==", bda + bfi, "sizeof(Buffer) = BUFFER_DATA_SIZE + BUFFER_FIELDS_SIZE");
    cmp_ok(bda, ">", 0, "BUFFER_DATA_SIZE (%d) > 0", bda);
    cmp_ok(bda, "<", bde, "BUFFER_DATA_SIZE (%d) < BUFFER_DESIRED_SIZE (%d)", bda, bde);
    cmp_ok(bfi, ">", 0, "BUFFER_FIELDS_SIZE (%d) > 0", bfi);
    cmp_ok(bfi, "<", bde, "BUFFER_FIELDS_SIZE (%d) < BUFFER_DESIRED_SIZE (%d)", bfi, bde);
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
        cmp_mem(r.ptr, tmp, r.len, "buffer_format_NUMBER %s => OK", tmp);
    }
}

static void print_and_compare(Buffer* buf, char* str, const char* fmt, ...) {
    va_list ap;
    va_list aq;
    va_start(ap, fmt);
    va_copy(aq, ap);

    vsprintf(str, fmt, ap);
    va_end(ap);

    buffer_clear(buf);
    buffer_format_vprint(buf, fmt, aq);
    va_end(aq);

    Slice r = buffer_get_slice(buf);
    cmp_mem(r.ptr, str, r.len, "buffer_format_print {%s} => OK", str);
}

static void test_format_print(void) {
    char str[1024];
    Buffer* buf = buffer_build();

    print_and_compare(buf, str, " Movie year %d, rating %.1f, name [%5.5s]", 1968, 8.3, "2001");
    print_and_compare(buf, str, "A more %-5.5s thing with pi=%7.4f", "complicated", M_PI);

    buffer_destroy(buf);
}

static void test_stack(void) {
    char str[1024];
    Buffer buf;
    buffer_init(&buf);

    print_and_compare(&buf, str, "This fits on the %s", "stack");
    print_and_compare(&buf, str, "This ALSO fits there");

    buffer_destroy(&buf);
}

static void test_stack_heap(void) {
    char str[1024];
    Buffer buf;
    buffer_init(&buf);

    const char* heap = "extremely useful heap";
    print_and_compare(&buf, str, "This is large enough to need the %s %s %s", heap, heap, heap);

    buffer_destroy(&buf);
}

static void test_pure_heap(void) {
    char str[1024];
    Buffer* buf = buffer_build();

    const char* heap = "humongously useful heap";
    print_and_compare(buf, str, "Also large enough to need the %s %s %s", heap, heap, heap);

    buffer_destroy(buf);
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
        ok(b->ptr != n->ptr, "buffer_clone: pointers %p and %p are different OK", b->ptr, n->ptr);
        cmp_ok(b->pos, "==", n->pos, "buffer_clone: lengths %d and %d are equal OK", b->pos, n->pos);
        cmp_mem(b->ptr, n->ptr, b->pos, "buffer_clone: %d bytes OK", b->pos);
        buffer_destroy(n);
        buffer_destroy(b);
    }
}

static void test_pack(void) {
    static struct {
        const char* s;
    } data[] = {
        { "you know it is there" },
        { "" },
        { "  this looks \t\t good, hopefully it is, because I am putting all my trust in it " },
    };
    for (unsigned int j = 0; j < ALEN(data); ++j) {
        Slice s = slice_wrap_string(data[j].s);
        Buffer* b = buffer_build();

        buffer_append_slice(b, s);
        cmp_ok(b->cap, ">=", b->pos, "buffer_pack: after append cap %d >= pos %d OK", b->cap, b->pos);

        buffer_pack(b);
        if (BUFFER_FLAG_CHK(b, BUFFER_FLAG_PTR_IN_HEAP)) {
            cmp_ok(b->cap, "==", b->pos, "buffer_pack: afer pack cap %d == pos %d OK", b->cap, b->pos);
        }

        buffer_clear(b);
        cmp_ok(b->pos, "==", 0, "buffer_pack: after clear pos %d OK", b->pos);

        buffer_pack(b);
        if (BUFFER_FLAG_CHK(b, BUFFER_FLAG_PTR_IN_HEAP)) {
            cmp_ok(b->cap, "==", 0, "buffer_pack: after clear+pack cap %d OK)", b->cap);
        }
        buffer_destroy(b);
    }
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_sizes();
    test_format_numbers();
    test_format_print();
    test_stack();
    test_stack_heap();
    test_pure_heap();
    test_clone();
    test_pack();

    done_testing();
}
