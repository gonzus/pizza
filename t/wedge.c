#include <string.h>
#include <tap.h>
#include "wedge.h"

static void test_char(void) {
    char c = 'Q';
    unsigned int l = 1;
    Wedge w; wedge_build_from_char(&w, c);
    ok(w.b != 0, "wedge from char DOES use a buffer");
    ok(w.s.len == l, "wedge's slice has correct length of %d bytes", l);
    ok(w.s.ptr[0] == c, "wedge's slice contains character [%c]", (int) c);
    ok(w.s.ptr[w.s.len] == '\0', "wedge's slice is null terminated");
    wedge_destroy(&w);
}

static void test_string(void) {
    const char* s = "what a load of baloney!";
    unsigned int l = strlen(s);
    Wedge w; wedge_build_from_string(&w, s);
    ok(w.b == 0, "wedge from string does NOT use a buffer");
    ok(w.s.len == l, "wedge's slice has correct length of %d bytes", l);
    ok(w.s.ptr == s, "wedge's slice points to string [%s]", s);
    ok(w.s.ptr[w.s.len] == '\0', "wedge's slice is null terminated");
    wedge_destroy(&w);
}

static void test_ptr_len_with_null_correct_length(void) {
    const char* s = "what a load of baloney!";
    unsigned int l = strlen(s);
    Wedge w; wedge_build_from_ptr_len(&w, s, l);
    ok(w.b == 0, "wedge from ptr/len with a null terminator does NOT use a buffer");
    ok(w.s.len == l, "wedge's slice has correct length of %d bytes", l);
    ok(w.s.ptr == s, "wedge's slice points to string [%s]", s);
    ok(w.s.ptr[w.s.len] == '\0', "wedge's slice is null terminated");
    wedge_destroy(&w);
}

static void test_ptr_len_with_null_too_long(void) {
    const char* s = "what a load of baloney!\0";
    unsigned int l = strlen(s);
    Wedge w; wedge_build_from_ptr_len(&w, s, l);
    ok(w.b == 0, "wedge from ptr/len too long and a null terminator does NOT use a buffer");
    ok(w.s.len == l, "wedge's slice has correct length of %d bytes", l);
    ok(w.s.ptr == s, "wedge's slice points to string [%s]", s);
    ok(w.s.ptr[w.s.len] == '\0', "wedge's slice is null terminated");
    wedge_destroy(&w);
}

static void test_ptr_len_without_null(void) {
    const char* s = "what a load of baloney!";
    unsigned int l = strlen(s) - 9;
    Wedge w; wedge_build_from_ptr_len(&w, s, l);
    ok(w.b != 0, "wedge from ptr/len too short and no null terminator DOES use a buffer");
    ok(w.s.len == l, "wedge's slice has correct length of %d bytes", l);
    ok(w.s.ptr != s, "wedge's slice does NOT point to string [%s]", s);
    ok(memcmp(w.s.ptr, s, l) == 0, "wedge's slice has same contents as string [%.*s]", l, s);
    ok(w.s.ptr[w.s.len] == '\0', "wedge's slice is null terminated");
    wedge_destroy(&w);
}

static void test_buffer_without_null(void) {
    const char* s = "what a load of baloney!";
    Buffer b; buffer_build(&b);
    buffer_append_string(&b, s, 0);
    Wedge w; wedge_build_from_buffer(&w, &b);
    ok(w.b != 0, "wedge from buffer without a null terminator DOES use a buffer");
    ok(w.s.len == b.len, "wedge's slice has correct length of %d bytes", b.len);
    ok(memcmp(w.s.ptr, b.ptr, b.len) == 0, "wedge's slice has same contents as buffer [%.*s]", b.len, b.ptr);
    ok(w.s.ptr[w.s.len] == '\0', "wedge's slice is null terminated");
    wedge_destroy(&w);
    buffer_destroy(&b);
}

static void test_buffer_with_null(void) {
    const char* s = "what a load of baloney!";
    unsigned int l = strlen(s);
    Buffer b; buffer_build(&b);
    buffer_append_string(&b, s, 0);
    buffer_null_terminate(&b);
    Wedge w; wedge_build_from_buffer(&w, &b);
    ok(w.b == 0, "wedge from buffer with a null terminator does NOT use a buffer");
    ok(w.s.len == l, "wedge's slice has correct length of %d bytes", l);
    ok(memcmp(w.s.ptr, s, l) == 0, "wedge's slice has same contents as buffer [%.*s]", l, s);
    ok(w.s.ptr[w.s.len] == '\0', "wedge's slice is null terminated");
    wedge_destroy(&w);
    buffer_destroy(&b);
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_char();
    test_string();
    test_ptr_len_with_null_correct_length();
    test_ptr_len_with_null_too_long();
    test_ptr_len_without_null();
    test_buffer_without_null();
    test_buffer_with_null();

    done_testing();
}
