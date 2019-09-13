#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>

#define LOG_SHOW 0

#define LOG(...) \
    if (LOG_SHOW) do { \
        fprintf(stderr, "LOG: "); \
        fprintf(stderr, __VA_ARGS__); \
    } while (0)

#endif
