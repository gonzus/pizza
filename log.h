#ifndef LOG_H_
#define LOG_H_

/*
 * Log -- debug- and run-time controllable logging
 *
 * Depending on the compile-time value of macro LOG_LEVEL_DEFAULT, some of the
 * calls to LOG_XXX will completely disappear from the code.
 *
 * TODO: The calls that remain can be controlled by the run-time value of
 * environment variable LOG_LEVEL.
 */

#define LOG_LEVEL_DEBUG      0
#define LOG_LEVEL_INFO       1
#define LOG_LEVEL_WARNING    2
#define LOG_LEVEL_ERROR      3
#define LOG_LEVEL_LAST       4

#define LOG_LEVEL_ENV "LOG_LEVEL"

#if !defined(LOG_LEVEL_DEFAULT)
#define LOG_LEVEL_DEFAULT LOG_LEVEL_WARNING
#endif

#if LOG_LEVEL_DEFAULT <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(...)   do { log_print_debug (__VA_ARGS__); } while (0)
#else
#define LOG_DEBUG(...)   do {} while (0)
#endif

#if LOG_LEVEL_DEFAULT <= LOG_LEVEL_INFO
#define LOG_INFO(...)    do { log_print_info   (__VA_ARGS__); } while (0)
#else
#define LOG_INFO(...)    do {} while (0)
#endif

#if LOG_LEVEL_DEFAULT <= LOG_LEVEL_WARNING
#define LOG_WARNING(...) do { log_print_warning(__VA_ARGS__); } while (0)
#else
#define LOG_WARNING(...) do {} while (0)
#endif

#if LOG_LEVEL_DEFAULT <= LOG_LEVEL_ERROR
#define LOG_ERROR(...)   do { log_print_error (__VA_ARGS__); } while (0)
#else
#define LOG_ERROR(...)   do {} while (0)
#endif

#define LOG(...) LOG_INFO(__VA_ARGS__)

void log_print_debug  (const char* fmt, ...);
void log_print_info   (const char* fmt, ...);
void log_print_warning(const char* fmt, ...);
void log_print_error  (const char* fmt, ...);

#endif
