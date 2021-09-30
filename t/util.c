#include <string.h>
#include <time.h>
#include <tap.h>
#include "buffer.h"
#include "util.h"

#define ALEN(a) (int) (sizeof(a) / sizeof(a[0]))

static void test_detailed_formatting(void) {
    static struct {
        unsigned long us;
        const char* formatted;
    } data[] = {
        { 0, "0us" },
        { 1, "1us" },
        { 10, "10us" },
        { 100, "100us" },
        { 1000, "1ms" },
        { 1005, "1ms5us" },
        { 1000000, "1s" },
        { 86400000000, "1D" },
        { 9*86400000000, "1W2D" },
        { 100*86400000000+4*3600000000+15*60000000+9*1000000, "14W2D4h15m9s" },
    };
    Buffer b; buffer_build(&b);
    for (int j = 0; j < ALEN(data); ++j) {
        unsigned long us = data[j].us;
        const char* expected = data[j].formatted;
        int elen = strlen(expected);
        buffer_clear(&b);
        format_detailed_us(us, &b);
        cmp_mem(b.ptr, expected, elen, "detailed formatted %lu us correctly as [%s] (%d:%.*s)", us, expected, elen, elen, b.ptr);
    }
    buffer_destroy(&b);
}

static void test_abbreviated_formatting(void) {
    static struct {
        unsigned long us;
        const char* formatted;
    } data[] = {
        { 0, "0.00us" },
        { 1, "1.00us" },
        { 10, "10.00us" },
        { 100, "100.00us" },
        { 1000, "1.00ms" },
        { 1500, "1.50ms" },
        { 3555000, "3.56s" },
        { 99900000000, "1.16D" },
        { 9*86400000000, "1.29W" },
        { 100*86400000000+4*3600000000+15*60000000+9*1000000, "14.31W" },
    };
    Buffer b; buffer_build(&b);
    for (int j = 0; j < ALEN(data); ++j) {
        unsigned long us = data[j].us;
        const char* expected = data[j].formatted;
        int elen = strlen(expected);
        buffer_clear(&b);
        format_abbreviated_us(us, &b);
        cmp_mem(b.ptr, expected, elen, "abbreviated formatted %lu us correctly as [%s] (%d:%.*s)", us, expected, elen, elen, b.ptr);
    }
    buffer_destroy(&b);
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_detailed_formatting();
    test_abbreviated_formatting();

    done_testing();
}
