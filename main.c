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

int main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_clone();
    test_pack();

    return 0;
}
