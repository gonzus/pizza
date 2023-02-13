#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "log.h"

#if !defined(LOG_USE_COLOR)
#define LOG_USE_COLOR 1
#endif

#if __APPLE__

int pthread_threadid_np(pthread_t thread, uint64_t *thread_id);
#define GETTID(T) pthread_threadid_np(NULL, &T)

#else

#include <sys/types.h>
#define GETTID(T) T = gettid()

#endif

static LogInfo log_info = {
    .level_compile_time = LOG_LEVEL_COMPILE_TIME,
    .level_run_time = -1,
    .skip_abort_on_error = 0,
    .skip_print_output = 0,
    .count[LOG_LEVEL_DEBUG] = 0,
    .count[LOG_LEVEL_INFO] = 0,
    .count[LOG_LEVEL_WARN] = 0,
    .count[LOG_LEVEL_ERROR] = 0,
    .count[LOG_LEVEL_FATAL] = 0,
};

static const char* log_level_name[LOG_LEVEL_LAST] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL",
};

static const char* log_level_label[LOG_LEVEL_LAST] = {
    "DBG",
    "INF",
    "WRN",
    "ERR",
    "FTL",
};

#if defined(LOG_USE_COLOR) && LOG_USE_COLOR > 0
static const char* log_color_reset = "\x1b[0m";
static const char* log_color_source = "\x1b[90m";
static const char* log_color_stamp = "\x1b[94m";
static const char* log_color_process = "\x1b[95m";
static const char* log_color_separator = "\x1b[91m";

static const char* log_level_color[LOG_LEVEL_LAST] = {
  "\x1b[32m",
  "\x1b[36m",
  "\x1b[33m",
  "\x1b[31m",
  "\x1b[35m",
};
#endif

static int log_get_runtime_level(void) {
    if (log_info.level_run_time < 0) {
        const char* str = getenv(LOG_LEVEL_ENV);
        int val = -1;
        if (str) {
            // try with log level name / label
            for (int j = 0; val < 0 && j < LOG_LEVEL_LAST; ++j) {
                if (strcmp(str, log_level_name[j]) == 0 ||
                    strcmp(str, log_level_label[j]) == 0) {
                    val = j;
                    break;
                }
            }
            // try with log level as a number
            if (val < 0) {
                val = strtol(str, 0, 10);
                if (val == 0 && errno == EINVAL) {
                    val = -1;
                }
            }
        }
        log_info.level_run_time = val < 0 ? LOG_LEVEL_COMPILE_TIME : val;
    }
    return log_info.level_run_time;
}

static void log_print(int level, const char* file, int line, int saved_errno, const char* fmt, va_list ap) {
    if (log_info.skip_print_output) {
        return;
    }

    time_t seconds = time(0);
    struct tm* local = localtime(&seconds);

    uint64_t pid = getpid();
    uint64_t tid = 0;
    GETTID(tid);

#if defined(LOG_USE_COLOR) && LOG_USE_COLOR > 0
    fprintf(stderr, "%s%04d/%02d/%02d %02d:%02d:%02d%s %s%ld %ld%s %s%-5s%s %s%s:%d%s"
            , log_color_stamp
            , local->tm_year + 1900, local->tm_mon + 1, local->tm_mday
            , local->tm_hour, local->tm_min, local->tm_sec
            , log_color_reset

            , log_color_process
            , (long int) pid, (long int) tid
            , log_color_reset

            , log_level_color[level]
            , log_level_name[level]
            , log_color_reset

            , log_color_source
            , file, line
            , log_color_reset
            );
    if (level >= LOG_LEVEL_ERROR) {
        if (saved_errno) {
            fprintf(stderr, " %s(errno %d: %s)%s"
                    , log_color_separator
                    , saved_errno, strerror(saved_errno)
                    , log_color_reset
                    );
        }
    }
    fprintf(stderr, " %s%s%s "
            , log_color_separator
            , "|"
            , log_color_reset
            );
#else
    fprintf(stderr, "%04d/%02d/%02d %02d:%02d:%02d %ld %ld %-5s %s:%d"
            , local->tm_year + 1900, local->tm_mon + 1, local->tm_mday
            , local->tm_hour, local->tm_min, local->tm_sec

            , (long int) pid, (long int) tid

            , log_level_name[level]

            , file, line
            );
    if (level >= LOG_LEVEL_ERROR) {
        if (saved_errno) {
            fprintf(stderr, " (errno %d: %s)", saved_errno, strerror(saved_errno));
        }
    }
    fprintf(stderr, " %s "
            , "|"
            );
#endif
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
}

void log_reset(int skip_abort_on_error, int skip_print_output) {
    log_info = (LogInfo) {
        .level_compile_time = LOG_LEVEL_COMPILE_TIME,
        .level_run_time = -1,
        .skip_abort_on_error = skip_abort_on_error,
        .skip_print_output = skip_print_output,
        .count[LOG_LEVEL_DEBUG] = 0,
        .count[LOG_LEVEL_INFO] = 0,
        .count[LOG_LEVEL_WARN] = 0,
        .count[LOG_LEVEL_ERROR] = 0,
        .count[LOG_LEVEL_FATAL] = 0,
    };
    log_get_runtime_level();
}

void log_print_debug(const char* file, int line, const char* fmt, ...) {
    if (log_get_runtime_level() > LOG_LEVEL_DEBUG) {
        return;
    }
    ++log_info.count[LOG_LEVEL_DEBUG];
    va_list ap;
    va_start(ap, fmt);
    log_print(LOG_LEVEL_DEBUG, file, line, 0, fmt, ap);
    va_end(ap);
}

void log_print_info(const char* file, int line, const char* fmt, ...) {
    if (log_get_runtime_level() > LOG_LEVEL_INFO) {
        return;
    }
    ++log_info.count[LOG_LEVEL_INFO];
    va_list ap;
    va_start(ap, fmt);
    log_print(LOG_LEVEL_INFO, file, line, 0, fmt, ap);
    va_end(ap);
}

void log_print_warn(const char* file, int line, const char* fmt, ...) {
    if (log_get_runtime_level() > LOG_LEVEL_WARN) {
        return;
    }
    ++log_info.count[LOG_LEVEL_WARN];
    va_list ap;
    va_start(ap, fmt);
    log_print(LOG_LEVEL_WARN, file, line, 0, fmt, ap);
    va_end(ap);
}

void log_print_error(const char* file, int line, const char* fmt, ...) {
    int saved_errno = errno;
    if (log_get_runtime_level() > LOG_LEVEL_ERROR) {
        return;
    }
    ++log_info.count[LOG_LEVEL_ERROR];
    va_list ap;
    va_start(ap, fmt);
    log_print(LOG_LEVEL_ERROR, file, line, saved_errno, fmt, ap);
    va_end(ap);
}

void log_print_fatal(const char* file, int line, const char* fmt, ...) {
    int saved_errno = errno;
    if (log_get_runtime_level() > LOG_LEVEL_FATAL) {
        return;
    }
    ++log_info.count[LOG_LEVEL_FATAL];
    va_list ap;
    va_start(ap, fmt);
    log_print(LOG_LEVEL_FATAL, file, line, saved_errno, fmt, ap);
    va_end(ap);

    if (log_info.skip_abort_on_error) {
        return;
    }

    abort();
}

const LogInfo* log_get_info(void) {
    return &log_info;
}
