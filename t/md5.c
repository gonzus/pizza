#include <tap.h>
#include "md5.h"

#define ALEN(a) (int) ((sizeof(a) / sizeof((a)[0])))

static void test_md5(void) {
    static struct {
        const char* str;
        const char* hash;
    } data[] = {
        {
            "",
            "d41d8cd98f00b204e9800998ecf8427e",
        },
        {
            "a",
            "0cc175b9c0f1b6a831c399e269772661",
        },
        {
            "abc",
            "900150983cd24fb0d6963f7d28e17f72",
        },
        {
            "message digest",
            "f96b697d7cb7938d525a2f31aaf161d0",
        },
        {
            "abcdefghijklmnopqrstuvwxyz",
            "c3fcd3d76192e4007dfb496cca67e13b",
        },
        {
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
            "d174ab98d277d9f5a5611c2c9f419d9f",
        },
        {
            "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
            "57edf4a22be3c955ac49da2e2107b67a",
        },
    };

    MD5 md5;
    Buffer b; buffer_build(&b);
    for (unsigned int j = 0; j < ALEN(data); ++j) {
        Slice str = slice_from_string(data[j].str, 0);
        Slice hash = slice_from_string(data[j].hash, 0);

        buffer_clear(&b);
        md5_compute(&md5, str, &b);

        ok(b.len == hash.len, "Got correct MD5 length [%d] == [%d] for [%d:%.*s%s]", b.len, hash.len, str.len, str.len > 50 ? 50 : str.len, str.ptr, str.len > 50 ? "..." : "");
        ok(slice_compare(buffer_slice(&b), hash) == 0, "Got correct MD5 hash [%.*s] for [%d:%.*s%s]", b.len, b.ptr, str.len, str.len > 50 ? 50 : str.len, str.ptr, str.len > 50 ? "..." : "");
    }
    buffer_destroy(&b);
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_md5();

    done_testing();
}
