#include <stdio.h>
#include <time.h>
#include "timer.h"

void timer_start(Timer* t) {
    gettimeofday(&t->t0, NULL);
}

void timer_stop(Timer* t) {
    gettimeofday(&t->t1, NULL);
}

unsigned long timer_elapsed_us(Timer* t)
{
    return ((t->t1.tv_sec  - t->t0.tv_sec ) * 1000000UL +
            (t->t1.tv_usec - t->t0.tv_usec));
}

void timer_format_elapsed(Timer* t, FILE* fp, int newline)
{
    static struct {
        unsigned long min;
        const char* name;
    } units[] = {
        { 24UL * 60UL * 60UL * 1000000UL, "D"  },
        {        60UL * 60UL * 1000000UL, "h"  },
        {               60UL * 1000000UL, "m"  },
        {                      1000000UL, "s"  },
        {                         1000UL, "ms" },
        {                            0UL, "us" },  // default, catch all
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
