#include "pizza/buffer.h"
#include "pizza/timer.h"
#include "pizza/util.h"

static struct {
    unsigned long min;
    const char* name;
} units[] = {
    { 7UL * 24UL * 60UL * 60UL * USECS_IN_A_SEC, "W"  },
    {       24UL * 60UL * 60UL * USECS_IN_A_SEC, "D"  },
    {              60UL * 60UL * USECS_IN_A_SEC, "h"  },
    {                     60UL * USECS_IN_A_SEC, "m"  },
    {                            USECS_IN_A_SEC, "s"  },
    {                           USECS_IN_A_MSEC, "ms" },
    { /* default, catch all */              0UL, "us" },
};

void format_detailed_us(unsigned long us, struct Buffer* b) {
    for (int j = 0; 1; ++j) {
        if (us < units[j].min) {
            continue;
        }
        unsigned long value = us;
        if (units[j].min == 0) {
            us = 0;
        } else {
            value /=  units[j].min;
            us -= value * units[j].min;
        }
        buffer_format_unsigned(b, value);
        buffer_append_string(b, units[j].name, -1);
        if (!us) {
            break;
        }
    }
}

void format_abbreviated_us(unsigned long us, struct Buffer* b) {
    for (int j = 0; 1; ++j) {
        if (us < units[j].min) {
            continue;
        }
        double value = us;
        if (units[j].min == 0) {
        } else {
            value /=  units[j].min;
        }
        buffer_format_print(b, "%.2f%s", value, units[j].name);
        break;
    }
}
