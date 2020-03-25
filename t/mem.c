#include <tap.h>
#include "mem.h"

static void test_mem_alloc_free(void) {
    typedef struct Foo {
        int bar;
        long baz;
    } Foo;

    const MemInfo* mem_info = 0;

    mem_info = mem_get_info();
    long cache_mem_alloc = mem_info->mem_alloc;
    long cache_mem_freed = mem_info->mem_freed;

    unsigned int fsize = sizeof(Foo);
    Foo* foo = 0;
    MEM_MALLOC(foo, Foo*, fsize);
    mem_info = mem_get_info();
    ok(foo != 0, "MEM_ALLOC allocated %d bytes", fsize);
    cmp_ok(mem_info->mem_alloc - cache_mem_alloc, "==", fsize, "total allocated bytes is %d bytes", fsize);
    cmp_ok(mem_info->mem_freed - cache_mem_freed, "==", 0, "total freed bytes is %d bytes", 0);
    MEM_FREE(foo, Foo*, fsize);
    mem_info = mem_get_info();
    ok(foo == 0, "MEM_FREE freed %d bytes", fsize);
    cmp_ok(mem_info->mem_alloc - cache_mem_alloc, "==", fsize, "total allocated bytes is %d bytes", fsize);
    cmp_ok(mem_info->mem_freed - cache_mem_freed, "==", fsize, "total freed bytes is %d bytes", fsize);
}

static void test_str_alloc_free_auto(void) {
    const MemInfo* mem_info = 0;

    mem_info = mem_get_info();
    long cache_mem_alloc = mem_info->mem_alloc;
    long cache_mem_freed = mem_info->mem_freed;

    const char* source = "This is my beatiful C null-terminated string";
    int len = strlen(source);

    char* copy = 0;
    MEM_STR_DUP(copy, source, 0);
    mem_info = mem_get_info();
    ok(copy != 0, "MEM_STR_DUP copied string with %d bytes -- automatic length", len);
    cmp_ok(mem_info->mem_alloc - cache_mem_alloc, "==", len+1, "total allocated bytes is %d+1 bytes", len);
    cmp_ok(mem_info->mem_freed - cache_mem_freed, "==", 0, "total freed bytes is %d bytes", 0);
    MEM_STR_FREE(copy, 0);
    mem_info = mem_get_info();
    ok(copy == 0, "MEM_STR_FREE freed %d+1 bytes", len);
    cmp_ok(mem_info->mem_alloc - cache_mem_alloc, "==", len+1, "total allocated bytes is %d+1 bytes", len);
    cmp_ok(mem_info->mem_freed - cache_mem_freed, "==", len+1, "total allocated bytes is %d+1 bytes", len);
}

static void test_str_alloc_free_manual(void) {
    const MemInfo* mem_info = 0;

    mem_info = mem_get_info();
    long cache_mem_alloc = mem_info->mem_alloc;
    long cache_mem_freed = mem_info->mem_freed;

    const char* source = "This is my beatiful C null-terminated string";
    int len = 17;

    char* copy = 0;
    MEM_STR_DUP(copy, source, len);
    mem_info = mem_get_info();
    ok(copy != 0, "MEM_STR_DUP copied string with %d bytes -- manual length", len);
    cmp_ok(mem_info->mem_alloc - cache_mem_alloc, "==", len+1, "total allocated bytes is %d+1 bytes", len);
    cmp_ok(mem_info->mem_freed - cache_mem_freed, "==", 0, "total freed bytes is %d bytes", 0);
    MEM_STR_FREE(copy, len);
    mem_info = mem_get_info();
    ok(copy == 0, "MEM_STR_FREE freed %d+1 bytes", len);
    cmp_ok(mem_info->mem_alloc - cache_mem_alloc, "==", len+1, "total allocated bytes is %d+1 bytes", len);
    cmp_ok(mem_info->mem_freed - cache_mem_freed, "==", len+1, "total allocated bytes is %d+1 bytes", len);
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_mem_alloc_free();
    test_str_alloc_free_auto();
    test_str_alloc_free_manual();

    done_testing();
}
