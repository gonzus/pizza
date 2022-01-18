#ifndef TIMER_H_
#define TIMER_H_

/*
 * Timer -- easier measuring of elapsed time
 */
#include <stdio.h>
#include <time.h>

#define NSECS_IN_A_USEC  1000UL
#define USECS_IN_A_MSEC  1000UL
#define MSECS_IN_A_SEC   1000UL
#define USECS_IN_A_SEC   (USECS_IN_A_MSEC * MSECS_IN_A_SEC)
#define NSECS_IN_A_SEC   (NSECS_IN_A_USEC * USECS_IN_A_SEC)

// We just use a pointer
struct Buffer;

typedef struct Timer {
    struct timespec ts0;
    struct timespec ts1;
} Timer;

void timer_start(Timer* t);
void timer_stop(Timer* t);

// Return elapsed time in ns
unsigned long timer_elapsed_ns(Timer* t);

// Return elapsed time in us
unsigned long timer_elapsed_us(Timer* t);

// Return elapsed time in ms
unsigned long timer_elapsed_ms(Timer* t);

// Return elapsed time in s
unsigned long timer_elapsed_s(Timer* t);

// Format elapsed time into a buffer using readable units
void timer_format_elapsed(Timer* t, struct Buffer* b);

#endif
