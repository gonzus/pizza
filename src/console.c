#include <stdio.h>
#include "pizza/util.h"
#include "pizza/stb_sprintf.h"
#include "pizza/console.h"

static char* vprint_cb(const char* buf, void* user, int len) {
    UNUSED_ARG(user);
    int nwritten = fwrite(buf, 1, len, stderr);
    if (nwritten != len) return 0;
    return (char*) buf;
}

void console_printf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    console_vprintf(fmt, ap);
    va_end(ap);
}

void console_vprintf(const char* fmt, va_list ap) {
    char buf[STB_SPRINTF_MIN];
    stbsp_vsprintfcb(vprint_cb, 0, buf, fmt, ap);
}
