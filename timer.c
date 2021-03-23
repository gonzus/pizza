#include <stdio.h>
#include <time.h>
#include "util.h"
#include "timer.h"

void timer_start(Timer* t) {
    gettimeofday(&t->t0, NULL);
}

void timer_stop(Timer* t) {
    gettimeofday(&t->t1, NULL);
}

unsigned long timer_elapsed_us(Timer* t) {
    return ((t->t1.tv_sec  - t->t0.tv_sec ) * USECS_IN_A_SEC +
            (t->t1.tv_usec - t->t0.tv_usec));
}

void timer_format_elapsed(Timer* t, struct Buffer* b) {
    unsigned long us = timer_elapsed_us(t);
    format_abbreviated_us(us, b);
}