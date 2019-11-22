#ifndef LOG_H_
#define LOG_H_

/*
 * Log -- debug- and run-time controllable logging
 *
 * Depending on the compile-time value of macro LOG_LEVEL, some of
 * the calls to LOG_XXX will completely disappear from the code.
 *
 * Depending on the run-time value of environment variable LOG_LEVEL, some of
 * the calls to LOG_XXX will remain in the code but become no-ops.
 */

#define LOG_LEVEL_DEBUG      0
#define LOG_LEVEL_INFO       1
#define LOG_LEVEL_WARNING    2
#define LOG_LEVEL_ERROR      3
#define LOG_LEVEL_LAST       4

// Compile-time default log level
#define LOG_LEVEL_DEFAULT LOG_LEVEL_WARNING

// Name of environment variable to control run-time logging.
#define LOG_LEVEL_ENV "LOG_LEVEL"

#if defined(LOG_LEVEL)
#define LOG_LEVEL_USE LOG_LEVEL
#else
#define LOG_LEVEL_USE LOG_LEVEL_DEFAULT
#endif

#if LOG_LEVEL_USE <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(...)   do { log_print_debug (LOG_LEVEL_USE, __VA_ARGS__); } while (0)
#else
#define LOG_DEBUG(...)   do {} while (0)
#endif

#if LOG_LEVEL_USE <= LOG_LEVEL_INFO
#define LOG_INFO(...)    do { log_print_info   (LOG_LEVEL_USE, __VA_ARGS__); } while (0)
#else
#define LOG_INFO(...)    do {} while (0)
#endif

#if LOG_LEVEL_USE <= LOG_LEVEL_WARNING
#define LOG_WARNING(...) do { log_print_warning(LOG_LEVEL_USE, __VA_ARGS__); } while (0)
#else
#define LOG_WARNING(...) do {} while (0)
#endif

#if LOG_LEVEL_USE <= LOG_LEVEL_ERROR
#define LOG_ERROR(...)   do { log_print_error (LOG_LEVEL_USE, __VA_ARGS__); } while (0)
#else
#define LOG_ERROR(...)   do {} while (0)
#endif

// A generic macro LOG that expands to LOG_INFO.
#define LOG(...) LOG_INFO(__VA_ARGS__)

// Implementations of the real logging functions, per level.
void log_print_debug  (int level, const char* fmt, ...);
void log_print_info   (int level, const char* fmt, ...);
void log_print_warning(int level, const char* fmt, ...);
void log_print_error  (int level, const char* fmt, ...);

#endif
