#ifndef LOG_H_
#define LOG_H_

#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO  1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_ERROR 3

#if !defined(LOG_LEVEL_DEFAULT)
#define LOG_LEVEL_DEFAULT LOG_LEVEL_WARN
#endif

#if LOG_LEVEL_DEFAULT <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(...) do { log_print_debug(__VA_ARGS__); } while (0)
#else
#define LOG_DEBUG(...) do {} while (0)
#endif

#if LOG_LEVEL_DEFAULT <= LOG_LEVEL_INFO
#define LOG_INFO(...) do { log_print_info (__VA_ARGS__); } while (0)
#else
#define LOG_INFO(...) do {} while (0)
#endif

#if LOG_LEVEL_DEFAULT <= LOG_LEVEL_WARN
#define LOG_WARN(...) do { log_print_warn (__VA_ARGS__); } while (0)
#else
#define LOG_WARN(...) do {} while (0)
#endif

#if LOG_LEVEL_DEFAULT <= LOG_LEVEL_ERROR
#define LOG_ERROR(...) do { log_print_error(__VA_ARGS__); } while (0)
#else
#define LOG_ERROR(...) do {} while (0)
#endif

#define LOG(...) LOG_INFO(__VA_ARGS__)

void log_print_debug(const char* fmt, ...);
void log_print_info (const char* fmt, ...);
void log_print_warn (const char* fmt, ...);
void log_print_error(const char* fmt, ...);

#endif
