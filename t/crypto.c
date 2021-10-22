    Slice p = slice_build_from_string("gonzo");
#include <string.h>
#include <tap.h>
#include "crypto.h"

#define ALEN(a) (int) ((sizeof(a) / sizeof((a)[0])))

static void test_decrypt(void) {
    uint8_t encrypted[] = {
    };
    uint8_t decrypted[] = {
    };

    uint32_t len = ALEN(encrypted);
    uint8_t iv[] = { 0x24, 0x73, 0x64, 0x65, 0x23, 0x28, 0x7d, 0x61 };
    Slice i = slice_build_from_ptr_len((const char*) iv, 8);

    Crypto crypto;
    crypto_init(&crypto, p, i);
    crypto_decrypt_cbc(&crypto, encrypted, len);

    ok(memcmp(encrypted, decrypted, len) == 0, "Could decrypt %d bytes with CBC Blowfish correctly", len);
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_decrypt();

    done_testing();
}
