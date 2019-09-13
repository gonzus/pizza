#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "log.h"

void log_print(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    time_t seconds = time(0);
    struct tm local;
    localtime_r(&seconds, &local);

    pid_t pid = getpid();

    fprintf(stderr, "%04d%02d%02d %02d%02d%02d %5ld %s ",
            local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
            local.tm_hour, local.tm_min, local.tm_sec,
            (long int) pid,
            "|");
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}
