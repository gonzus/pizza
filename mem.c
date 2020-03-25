#include <stdio.h>
#include "mem.h"

static MemInfo mem_info;

static long mem_called_something(const char* file,
        int line,
        void* var,
        int count,
        long size,
        const char* label);

void mem_init(void) {
    if (mem_info.mem_inited) {
        return;
    }
    mem_info.mem_inited = 1;

#if defined(MEM_CHECK) && MEM_CHECK >= 1
    atexit(mem_fini);
    mem_info.mem_alloc = mem_info.mem_freed = 0;
    fprintf(stderr, "=== MEM BEG %ld %ld ===\n", mem_info.mem_alloc, mem_info.mem_freed);
#endif
}

void mem_fini(void) {
    if (!mem_info.mem_inited) {
        return ;
    }
    mem_info.mem_inited = 0;

#if defined(MEM_CHECK) && MEM_CHECK >= 1
    fprintf(stderr, "=== MEM END %ld %ld ===\n", mem_info.mem_alloc, mem_info.mem_freed);
    if (mem_info.mem_alloc == mem_info.mem_freed) {
        fprintf(stderr, "=== MEM OK ===\n");
    } else {
        fprintf(stderr, "=== MEM ERR %ld BYTES ===\n", mem_info.mem_alloc - mem_info.mem_freed);
    }
#endif
}

void mem_called_alloc(const char* file,
        int line,
        void* var,
        int count,
        long size) {
    mem_info.mem_alloc += mem_called_something(file, line, var, count, size, "MAL");
}

void mem_called_free(const char* file,
        int line,
        void* var,
        int count,
        long size) {
    mem_info.mem_freed += mem_called_something(file, line, var, count, size, "FRE");
}

const MemInfo* mem_get_info(void) {
    return &mem_info;
}

static long mem_called_something(const char* file,
        int line,
        void* var,
        int count,
        long size,
        const char* label) {
    long total = 0;

#if defined(MEM_CHECK) && MEM_CHECK >= 1
    if (!var) {
        return 0;
    }
    if (size <= 0 || count <= 0) {
        return 0;
    }

    if (!mem_info.mem_inited) {
        mem_init();
    }
    total = size * count;

#if defined(MEM_CHECK) && MEM_CHECK >= 2
    fprintf(stderr, "=== MEM %s %s %d %p %d %ld %ld ===\n",
            label, file, line, var, count, size, total);
#else
    (void) file;
    (void) line;
    (void) label;
#endif

#else
    (void) file;
    (void) line;
    (void) label;
    (void) var;
    (void) count;
    (void) size;
#endif

    return total;
}
