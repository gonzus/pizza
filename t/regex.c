#include <string.h>
#include <tap.h>
#include "regex.h"

#define ALEN(a) (int) ((sizeof(a) / sizeof((a)[0])))

static void test_simple(void) {
    static struct Sample {
        const char* source;
        int match;
        const char* string;
    } samples[] = {
        { "^[0-9]+$", 0, 0 },
        { 0, 1, "0" },
        { 0, 1, "1" },
        { 0, 1, "123" },
        { 0, 0, "" },
        { 0, 0, "abc" },

        { "^[_a-zA-Z][_a-zA-Z0-9]*$", 0, 0 },
        { 0, 1, "j" },
        { 0, 1, "Maybe" },
        { 0, 1, "_LINE_" },
        { 0, 0, "" },
        { 0, 0, "7" },
        { 0, 0, "9pqr" },
        { 0, 0, "a-b-c" },

        { "^([0-9]+|[a-z]+)$", 0, 0 },
        { 0, 1, "3" },
        { 0, 1, "77" },
        { 0, 1, "q" },
        { 0, 1, "abc" },
        { 0, 0, "" },
        { 0, 0, "a9" },
        { 0, 0, "8w" },
    };
    ReCompiled *rx = 0;
    for (int j = 0; j < ALEN(samples); ++j) {
        if (samples[j].source) {
            if (rx) {
                re_free(rx);
                rx = 0;
                ok(!rx, "previous regex successfully destroyed");
            }
            const char* ret = re_compile(&rx, samples[j].source, 0);
            ok(!ret, "regex {%s} successfully compiled", samples[j].source);
            ok(!!rx, "compiled regex for {%s} is not null", samples[j].source);
        }
        if (!samples[j].string) continue;

        int len = strlen(samples[j].string);
        int matched = re_match(rx, (const unsigned char*) samples[j].string, len);
        ok(!!matched == !!samples[j].match, "regex %s match {%s}", samples[j].match ? "does" : "does not", samples[j].string);
    }
    if (rx) {
        re_free(rx);
        rx = 0;
        ok(!rx, "final regex successfully destroyed");
    }
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_simple();

    done_testing();
}
