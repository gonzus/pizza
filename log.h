#ifndef LOG_H_
#define LOG_H_

#if !defined(LOG_SHOW)
#define LOG_SHOW 0
#endif

#define LOG(...) \
    if (LOG_SHOW) do { \
        log_print(__VA_ARGS__); \
    } while (0)

void log_print(const char* fmt, ...);

#endif
