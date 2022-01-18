#include <tap.h>
#include "pizza/thrpool.h"

#define NUM_THREADS 4

typedef struct Range {
    unsigned long lo;
    unsigned long hi;
    unsigned long sum;
} Range;

static void range_adder(void* arg) {
    Range* r = (Range*) arg;
    r->sum = 0;
    for (unsigned long j = r->lo; j <= r->hi; ++j) {
        r->sum += j;
    }
}

static void test_thrpool_sum_range(void) {
    int rc = 0;
    unsigned long top = 50000000;
    unsigned long size = (top + NUM_THREADS - 1) / NUM_THREADS;
    unsigned long lo = 1;
    Range range[NUM_THREADS];
    ThrPool* pool = thrpool_create(NUM_THREADS, 1024);
    ok(pool != 0, "could create thrpool");
    for (int j = 0; j < NUM_THREADS; j++) {
        unsigned long hi = lo + size - 1;
        if (hi > top) {
            hi = top;
        }
        range[j] = (Range) { .lo = lo, .hi = hi, .sum = 0 };
        rc = thrpool_add(pool, range_adder, &range[j]);
        cmp_ok(rc, "==", THRPOOL_STATUS_OK, "range %lu..%lu added to thrpool", lo, hi);
        lo = hi + 1;
    };
    rc = thrpool_destroy(pool, 0);
    cmp_ok(rc, "==", THRPOOL_STATUS_OK, "could destroy thrpool");

    unsigned long expected = top * (top + 1) / 2;
    unsigned long got = 0;
    for (int j = 0; j < NUM_THREADS; j++) {
        got += range[j].sum;
    };

    cmp_ok(got, "==", expected, "sum of 1..%lu ends up being %lu", top, expected);
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_thrpool_sum_range();

    done_testing();
}
