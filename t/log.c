#include <stdlib.h>
#include <tap.h>
#include "pizza/log.h"

static void test_log_error_dies(void) {
    log_reset(0, 0);
    dies_ok({ LOG_ERROR("DAMN!"); }, "LOG_ERROR dies as expected");
}

static void test_levels(void) {
    for (int r = 0; r < LOG_LEVEL_LAST; ++r) {
        cmp_ok(r, "<", LOG_LEVEL_LAST, "--- Checking runtime level %d ---", r);

        char level[50];
        sprintf(level, "%d", r);
        setenv(LOG_LEVEL_ENV, level, 1);
        log_reset(1, 1);

        for (int l = 0; l < LOG_LEVEL_LAST; ++l) {
            LogInfo saved = *log_get_info();
            switch (l) {
                case LOG_LEVEL_DEBUG:
                    LOG_DEBUG("LOG_DEBUG %d", r);
                    break;
                case LOG_LEVEL_INFO:
                    LOG_INFO("LOG_INFO %d", r);
                    break;
                case LOG_LEVEL_WARN:
                    LOG_WARN("LOG_WARN%d", r);
                    break;
                case LOG_LEVEL_ERROR:
                    LOG_ERROR("LOG_ERROR %d", r);
                    break;
            }

            int active = saved.level_compile_time <= l && saved.level_run_time <= l;
            const LogInfo* current = log_get_info();
            for (int t = 0; t < LOG_LEVEL_LAST; ++t) {
                int delta = active && l == t ? 1 : 0;
                cmp_ok(current->count[t], "==", saved.count[t] + delta,
                       "Runtime %d: level %d => count for %d changed %d OK", r, l, t, delta);
            }
        }
    }
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_levels();
    test_log_error_dies();

    done_testing();
}
