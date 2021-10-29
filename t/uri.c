#include <string.h>
#include <tap.h>
#include "uri.h"

#define ALEN(a) (int) ((sizeof(a) / sizeof((a)[0])))

static void test_encode(void) {
    static struct {
        const char* label;
        const char* plain;
        const char* encoded;
    } data[] = {
        { "reserved", "!#$%&'()*+,/:;=?@[]", "%21%23%24%25%26%27%28%29%2a%2b%2c%2f%3a%3b%3d%3f%40%5b%5d" },
        { "mixed", "My name (yes, my name) is Bond!", "My%20name%20%28yes%2c%20my%20name%29%20is%20Bond%21" },
        { "clean", "Mississippi", "Mississippi" },
        { "no plus", "me and you", "me%20and%20you" },
    };

    Buffer b; buffer_build(&b);
    for (unsigned int j = 0; j < ALEN(data); ++j) {
        Slice plain = slice_from_string(data[j].plain, 0);
        Slice ex_encoded = slice_from_string(data[j].encoded, 0);

        buffer_clear(&b);
        uint32_t len = uri_encode(plain, &b);
        Slice encoded = buffer_slice(&b);
        ok(len == ex_encoded.len, "For %s got %d == %d bytes while URI encoding", data[j].label, len, ex_encoded.len);
        ok(slice_compare(encoded, ex_encoded) == 0, "Could URI encode %s [%d:%.*s%s]", data[j].label, encoded.len, encoded.len > 50 ? 50 : encoded.len, encoded.ptr, encoded.len > 50 ? "..." : "");
    }
    buffer_destroy(&b);
}

static void test_decode(void) {
    static struct {
        const char* label;
        const char* encoded;
        const char* plain;
    } data[] = {
        { "reserved", "%21%23%24%25%26%27%28%29%2a%2b%2c%2f%3a%3b%3d%3f%40%5b%5d", "!#$%&'()*+,/:;=?@[]" },
        { "mixed", "My%20name%20%28yes%2c%20my%20name%29%20is%20Bond%21", "My name (yes, my name) is Bond!" },
        { "clean", "Mississippi", "Mississippi" },
        { "no plus", "me%20and%20you", "me and you" },
    };

    Buffer b; buffer_build(&b);
    for (unsigned int j = 0; j < ALEN(data); ++j) {
        Slice encoded = slice_from_string(data[j].encoded, 0);
        Slice ex_plain = slice_from_string(data[j].plain, 0);

        buffer_clear(&b);
        uint32_t len = uri_decode(encoded, &b);
        Slice plain = buffer_slice(&b);
        ok(len == ex_plain.len, "For %s got %d == %d bytes while URI decoding", data[j].label, len, ex_plain.len);
        ok(slice_compare(ex_plain, plain) == 0, "Could URI decode %s [%d:%.*s%s]", data[j].label, plain.len, plain.len > 50 ? 50 : plain.len, plain.ptr, plain.len > 50 ? "..." : "");
    }
    buffer_destroy(&b);
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_encode();
    test_decode();

    done_testing();
}
