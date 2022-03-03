#include "pizza/util.h"
#include "pizza/timer.h"

void timer_start(Timer* t) {
    clock_gettime(CLOCK_MONOTONIC, &t->ts0);
    t->ts1 = t->ts0;
}

void timer_stop(Timer* t) {
    clock_gettime(CLOCK_MONOTONIC, &t->ts1);
}

unsigned long timer_elapsed_ns(Timer* t) {
    return ((t->ts1.tv_sec  - t->ts0.tv_sec ) * NSECS_IN_A_SEC +
            (t->ts1.tv_nsec - t->ts0.tv_nsec));
}

unsigned long timer_elapsed_us(Timer* t) {
    return timer_elapsed_ns(t) / NSECS_IN_A_USEC;
}

unsigned long timer_elapsed_ms(Timer* t) {
    return timer_elapsed_us(t) / USECS_IN_A_MSEC;
}

unsigned long timer_elapsed_s(Timer* t) {
    return timer_elapsed_ms(t) / MSECS_IN_A_SEC;
}

void timer_format_elapsed(Timer* t, Buffer* b) {
    unsigned long us = timer_elapsed_us(t);
    format_abbreviated_us(us, b);
}
