#include <string.h>
#include <tap.h>
#include "util.h"
#include "buffer.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define ALEN(a) (int) (sizeof(a) / sizeof(a[0]))

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
    for (int j = 0; j < ALEN(data); ++j) {
        char tmp[1024];
        Buffer b; buffer_build(&b);
        switch (data[j].t) {
            case 0:
                buffer_format_unsigned(&b, data[j].u);
                sprintf(tmp, "%llu", data[j].u);
                break;
            case 1:
                buffer_format_signed(&b, data[j].s);
                sprintf(tmp, "%lld", data[j].s);
                break;
            case 2:
                buffer_format_double(&b, data[j].d);
                sprintf(tmp, "%f", data[j].d);
                break;
            default:
                continue;
        }

        cmp_mem(b.ptr, tmp, b.len, "buffer_format_NUMBER %s => OK", tmp);
        buffer_destroy(&b);
    }
}

static void print_and_compare(Buffer* b, char* str, const char* fmt, ...) {
    va_list ap;
    va_list aq;
    va_start(ap, fmt);
    va_copy(aq, ap);

    vsprintf(str, fmt, ap);
    va_end(ap);

    buffer_clear(b);
    buffer_format_vprint(b, fmt, aq);
    va_end(aq);

    cmp_mem(b->ptr, str, b->len, "buffer_format_print {%s} => OK", str);
}

static void test_format_print(void) {
    char str[1024];
    Buffer b; buffer_build(&b);

    print_and_compare(&b, str, " Movie year %d, rating %.1f, name [%5.5s]", 1968, 8.3, "2001");
    print_and_compare(&b, str, "A more %-5.5s thing with pi=%7.4f", "complicated", M_PI);

    buffer_destroy(&b);
}

static void test_stack(void) {
    char str[1024];
    Buffer b; buffer_build(&b);

    print_and_compare(&b, str, "This fits on the %s", "stack");
    print_and_compare(&b, str, "This ALSO fits there");

    buffer_destroy(&b);
}

static void test_stack_heap(void) {
    char str[1024];
    Buffer b; buffer_build(&b);

    const char* heap = "extremely useful heap";
    print_and_compare(&b, str, "This is large enough to need the %s %s %s", heap, heap, heap);

    buffer_destroy(&b);
}

static void test_pure_heap(void) {
    char str[1024];
    Buffer* b = buffer_allocate();

    const char* heap = "humongously useful heap";
    print_and_compare(b, str, "Also large enough to need the %s %s %s", heap, heap, heap);

    buffer_release(b);
}

static void test_clone(void) {
    static struct {
        const char* s;
    } data[] = {
        { "you know it is there" },
        { "" },
        { "  this looks \t\t good " },
    };
    for (int j = 0; j < ALEN(data); ++j) {
        const char* ptr = data[j].s;
        uint32_t len = strlen(data[j].s);
        Buffer b; buffer_build(&b);
        buffer_append_string(&b, ptr, len);
        Buffer* n = buffer_clone(&b);
        ok(b.ptr != n->ptr, "buffer_clone: pointers %p and %p are different OK", b.ptr, n->ptr);
        cmp_ok(b.len, "==", n->len, "buffer_clone: lengths %d and %d are equal OK", b.len, n->len);
        cmp_mem(b.ptr, n->ptr, b.len, "buffer_clone: %d bytes OK", b.len);
        buffer_release(n);
        buffer_destroy(&b);
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
    for (int j = 0; j < ALEN(data); ++j) {
        const char* ptr = data[j].s;
        uint32_t len = strlen(data[j].s);
        Buffer b; buffer_build(&b);

        buffer_append_string(&b, ptr, len);
#if 0
        dump_bytes(stderr, b.ptr, b.len);
#endif
        cmp_ok(b.cap, ">=", b.len, "buffer_pack: after append cap %d >= len %d OK", b.cap, b.len);

        buffer_pack(&b);
        if (BUFFER_FLAG_CHK(&b, BUFFER_FLAG_PTR_IN_HEAP)) {
            cmp_ok(b.cap, "==", b.len, "buffer_pack: afer pack cap %d == len %d OK", b.cap, b.len);
        }

        buffer_clear(&b);
        cmp_ok(b.len, "==", 0, "buffer_pack: after clear len %d OK", b.len);

        buffer_pack(&b);
        if (BUFFER_FLAG_CHK(&b, BUFFER_FLAG_PTR_IN_HEAP)) {
            cmp_ok(b.cap, "==", 0, "buffer_pack: after clear+pack cap %d OK)", b.cap);
        }
        buffer_destroy(&b);
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
