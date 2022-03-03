#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <stdarg.h>

/*
 */

void console_printf(const char* fmt, ...);
void console_vprintf(const char* fmt, va_list ap);

#endif
