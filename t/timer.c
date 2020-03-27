#define _POSIX_C_SOURCE 199309L // for nanosleep
#include <time.h>
#include <tap.h>
#include "timer.h"

#define ALEN(a) (int) (sizeof(a) / sizeof(a[0]))

#define S_TO_NS  1000000000UL
#define MS_TO_NS 1000000UL
#define US_TO_NS 1000UL

static void test_timer(void) {
    static unsigned long pauses_ms[] = {
        1,
        5,
        10,
        50,
        100,
        500,
        1000,
    };
    for (int j = 0; j < ALEN(pauses_ms); ++j) {
        unsigned long pause_ns = pauses_ms[j] * MS_TO_NS;
        struct timespec pause;
        pause.tv_nsec = pause_ns % S_TO_NS;
        pause.tv_sec  = pause_ns / S_TO_NS;

        Timer t;
        timer_start(&t);
        nanosleep(&pause, 0);
        timer_stop(&t);
        unsigned long us = timer_elapsed_us(&t);

        cmp_ok(us * US_TO_NS, ">=", pause_ns, "slept for %lu us which is at least %d ns", us, pause_ns);
    }
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_timer();

    done_testing();
}
