#include <string.h>
#include <tap.h>
#include "log.h"
#include "regex.h"

#define ALEN(a) (int) ((sizeof(a) / sizeof((a)[0])))

static void test_simple(void) {
  static struct Sample {
    const char* source;
    int match;
    const char* text;
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
  Regex rx; regex_build(&rx);
  for (int j = 0; j < ALEN(samples); ++j) {
    int err = 0;
    if (samples[j].source) {
      Slice s = slice_from_string(samples[j].source, 0);
      err = regex_compile(&rx, s, 0);
      LOG_DEBUG("compiled {%.*s} => %d [%s]", s.len, s.ptr, err, rx.zErr ? rx.zErr : "");
      ok(!err, "regex {%.*s} successfully compiled", s.len, s.ptr);
    }
    if (samples[j].text) {
      int matched = 0;
      Slice t = slice_from_string(samples[j].text, 0);
      err = regex_match(&rx, t, &matched);
      LOG_DEBUG("matched {%.*s} => %d - %d [%s]", t.len, t.ptr, matched, err, rx.zErr ? rx.zErr : "");
      ok(!err, "regex successfully matched against {%.*s}", t.len, t.ptr);
      ok(!!matched == !!samples[j].match, "regex %s match {%.*s}", samples[j].match ? "does" : "does not", t.len, t.ptr);
    }
  }
  regex_destroy(&rx);
}

static void test_invalid(void) {
  static struct Sample {
    const char* source;
    int err;
  } samples[] = {
    { "a$b", RX_UNRECOGNIZED_CHARACTER },
    { "\\z", RX_UNKNOWN_ESCAPE },
    { "[[:digit:]]", RX_UNKNOWN_POSIX_CHARCLASS },
    { "(a", RX_UNMATCHED_PARENTHESIS },
    { "a{2,3", RX_UNMATCHED_BRACE },
    { "[a", RX_UNMATCHED_BRACKET },
    { "*", RX_STAR_WITHOUT_OPERAND },
    { "+", RX_PLUS_WITHOUT_OPERAND },
    { "?", RX_QUESTION_WITHOUT_OPERAND },
    { "{1,2}", RX_LOHI_WITHOUT_OPERAND },
    { "a{3,2}", RX_LOHI_HI_SMALLER_THAN_LO },
    { "a{0,0}", RX_LOHI_BOTH_ZERO },
  };
  Regex rx; regex_build(&rx);
  for (int j = 0; j < ALEN(samples); ++j) {
    int err = 0;
    Slice s = slice_from_string(samples[j].source, 0);
    err = regex_compile(&rx, s, 0);
    // LOG_INFO("compiled {%.*s} => %d [%s]", s.len, s.ptr, err, regex_error(err));
    int expected = samples[j].err;
    ok(err == expected, "regex {%.*s} compiled with correct error %d [%s]", s.len, s.ptr, expected, regex_error(expected));
  }
  regex_destroy(&rx);
}

int main (int argc, char* argv[]) {
  (void) argc;
  (void) argv;

  test_simple();
  test_invalid();

  done_testing();
}
