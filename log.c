#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "log.h"

static const char* log_level_label[] = {
    "DBG",
    "INF",
    "WRN",
    "ERR",
};

static void log_print(int level, const char* fmt, va_list ap) {
    int saved_errno = errno;

    time_t seconds = time(0);
    struct tm local;
    localtime_r(&seconds, &local);

    pid_t pid = getpid();

    fprintf(stderr, "[%.3s] %04d%02d%02d %02d%02d%02d %5ld | ",
            log_level_label[level],
            local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
            local.tm_hour, local.tm_min, local.tm_sec,
            (long int) pid);
    if (level >= LOG_LEVEL_WARN) {
        if (errno) {
            fprintf(stderr, "(%d) %s | ", saved_errno, strerror(saved_errno));
        } else {
            fprintf(stderr, "%s | ", strerror(saved_errno));
        }
    }
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");

    if (level >= LOG_LEVEL_ERROR) {
        abort();
    }
}

void log_print_debug(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    log_print(LOG_LEVEL_DEBUG, fmt, ap);
    va_end(ap);
}

void log_print_info (const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    log_print(LOG_LEVEL_INFO , fmt, ap);
    va_end(ap);
}

void log_print_warn (const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    log_print(LOG_LEVEL_WARN , fmt, ap);
    va_end(ap);
}

void log_print_error(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    log_print(LOG_LEVEL_ERROR, fmt, ap);
    va_end(ap);
}
