#ifndef TIMER_H_
#define TIMER_H_

/*
 * Timer -- easier measuring of elapsed time
 */
#include <stdio.h>
#include <sys/time.h>

#define USECS_IN_A_MSEC  1000UL
#define USECS_IN_A_SEC   (USECS_IN_A_MSEC * USECS_IN_A_MSEC)

// We just use a pointer
struct Buffer;

typedef struct Timer {
    struct timeval t0;
    struct timeval t1;
} Timer;

void timer_start(Timer* t);
void timer_stop(Timer* t);

// Return elapsed time in us
unsigned long timer_elapsed_us(Timer* t);

// Format elapsed time into a buffer using readable units
void timer_format_elapsed(Timer* t, struct Buffer* b);

#endif
