#include <stdio.h>
#include <time.h>
#include "timer.h"

#define USECS_IN_A_MSEC  1000UL
#define USECS_IN_A_SEC   (USECS_IN_A_MSEC * USECS_IN_A_MSEC)

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

void timer_format_elapsed(Timer* t, FILE* fp, int newline) {
    static struct {
        unsigned long min;
        const char* name;
    } units[] = {
        { 24UL * 60UL * 60UL * USECS_IN_A_SEC, "D"  },
        {        60UL * 60UL * USECS_IN_A_SEC, "h"  },
        {               60UL * USECS_IN_A_SEC, "m"  },
        {                      USECS_IN_A_SEC, "s"  },
        {                     USECS_IN_A_MSEC, "ms" },
        { /* default, catch all */        0UL, "us" },
    };
    unsigned long us = timer_elapsed_us(t);
    for (int j = 0; 1; ++j) {
        if (us > units[j].min) {
            float value = us / (float) units[j].min;
            fprintf(fp, "%.2f %s", value, units[j].name);
            break;
        }
    }

    if (newline) {
        fputc('\n', fp);
    }
}
