#include <stdio.h>
#include "mem.h"

int mem_unused = 0;

#if defined(MEM_CHECK) && MEM_CHECK >= 1

long mem_alloc = 0;
long mem_freed = 0;

static int mem_inited = 0;

void mem_init(void) {
    if (mem_inited) {
        return;
    }

    atexit(mem_fini);

    mem_inited = 1;
    mem_alloc = mem_freed = 0;

#if defined(MEM_CHECK) && MEM_CHECK >= 1
    fprintf(stderr, "=== MEM BEG %ld %ld ===\n", mem_alloc, mem_freed);
#endif
}

void mem_fini(void) {
    if (!mem_inited) {
        return ;
    }

#if defined(MEM_CHECK) && MEM_CHECK >= 1
    fprintf(stderr, "=== MEM END %ld %ld ===\n", mem_alloc, mem_freed);
    if (mem_alloc == mem_freed) {
        fprintf(stderr, "=== MEM OK ===\n");
    } else {
        fprintf(stderr, "=== MEM ERR %ld BYTES ===\n", mem_alloc - mem_freed);
    }
#endif
    mem_inited = 0;
}

int mem_called_alloc(const char* file,
        int line,
        void* var,
        int count,
        long size) {
    if (!var) {
        return 0;
    }
    if (size <= 0 || count <= 0) {
        return 0;
    }

    if (!mem_inited) {
        mem_init();
    }

    long total = size * count;

#if defined(MEM_CHECK) && MEM_CHECK >= 2
    fprintf(stderr, "=== MEM MAL %s %d %p %d %ld %ld ===\n",
            file, line, var, count, size, total);
#else
    (void) file;
    (void) line;
#endif

    mem_alloc += total;
    return total;
}

int mem_called_free(const char* file,
        int line,
        void* var,
        int count,
        long size) {
    if (!var) {
        return 0;
    }
    if (size <= 0 || count <= 0) {
        return 0;
    }

    if (!mem_inited) {
        mem_init();
    }

    long total = size * count;

#if defined(MEM_CHECK) && MEM_CHECK >= 2
    fprintf(stderr, "=== MEM FRE %s %d %p %d %ld %ld ===\n",
            file, line, var, count, size, total);
#else
    (void) file;
    (void) line;
#endif

    mem_freed += total;
    return total;
}

#else

void mem_init(void) {
}

void mem_fini(void) {
}

#endif /* #if defined(MEM_CHECK) && MEM_CHECK >= 1 */
