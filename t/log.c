#include <stdlib.h>
#include <tap.h>
#include "log.h"

static void test_log_fatal_dies(void) {
  log_reset(0, 1);
  dies_ok({ LOG_FATAL("sic transit gloria mundi"); }, "LOG_FATAL dies as expected");
}

static void test_levels(void) {
  for (int level = 0; level <= LOG_LEVEL_ERROR; ++level) {
    cmp_ok(level, "<", LOG_LEVEL_LAST, "--- Checking runtime level %d ---", level);

    char level_str[50];
    sprintf(level_str, "%d", level);
    setenv(LOG_LEVEL_ENV, level_str, 1);
    log_reset(1, 1);

    for (int what = 0; what <= LOG_LEVEL_ERROR; ++what) {
      LogInfo saved = *log_get_info();
      switch (what) {
        case LOG_LEVEL_DEBUG:
          LOG_DEBUG("LOG_DEBUG %d", level);
          break;
        case LOG_LEVEL_INFO:
          LOG_INFO("LOG_INFO %d", level);
          break;
        case LOG_LEVEL_WARN:
          LOG_WARN("LOG_WARN%d", level);
          break;
        case LOG_LEVEL_ERROR:
          LOG_ERROR("LOG_ERROR %d", level);
          break;
        default:
          ok(0, "Invalid log level %d", what);
      }

      int active = saved.level_compile_time <= what && saved.level_run_time <= what;
      const LogInfo* current = log_get_info();
      for (int expected = 0; expected <= LOG_LEVEL_ERROR; ++expected) {
        int expected_diff = active && what == expected ? 1 : 0;
        cmp_ok(current->count[expected], "==", saved.count[expected] + expected_diff,
               "Runtime %d: level %d => count for %d changed %d OK", level, what, expected, expected_diff);
      }
    }
  }
}

int main (int argc, char* argv[]) {
  (void) argc;
  (void) argv;

  test_levels();
  test_log_fatal_dies();

  done_testing();
}
