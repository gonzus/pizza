#include <string.h>
#include <tap.h>
#include "memory.h"
#include "buffer.h"
#include "crypto.h"

#define ALEN(a) (int) ((sizeof(a) / sizeof((a)[0])))

static void test_crypto(void) {
    static struct {
        const char* label;
        uint32_t elen;
        uint8_t eptr[16];
        uint32_t dlen;
        uint8_t dptr[16];
    } data[] = {
        {
            "partially filled last block",
            16, {
                0x6c, 0x54, 0x31, 0xab, 0xc2, 0x6a, 0x42, 0x0c,
                0x35, 0xed, 0x77, 0xb9, 0x8d, 0x32, 0xa0, 0xe2,
            },
            10, {
                0x78, 0x9c, 0x01, 0x7f, 0x00, 0x80, 0xff, 0x3d,
                0x2e, 0x2e,
            },
        },
        {
            "fully filled last block",
            16, {
                0x6c, 0x54, 0x31, 0xab, 0xc2, 0x6a, 0x42, 0x0c,
                0x83, 0xd9, 0xa4, 0xaf, 0x29, 0x8a, 0x00, 0x62,
            },
            8, {
                0x78, 0x9c, 0x01, 0x7f, 0x00, 0x80, 0xff, 0x3d,
            },
        },
    };

    Slice p = slice_build_from_string("my beautiful passphrase");
    uint8_t iv[] = { 0xde, 0xad, 0xbe, 0xef, 0xab, 0xad, 0xca, 0xfe };
    Slice i = slice_build_from_ptr_len((const char*) iv, 8);

    Crypto crypto;
    crypto_init(&crypto, p, i);

    Buffer b; buffer_build(&b);
    for (uint32_t j = 0; j < ALEN(data); ++j) {
#if 0
        uint8_t buf[16];
        uint32_t len = data[j].dlen;
        memcpy(buf, data[j].dptr, len);
        len = crypto_encrypt_cbc(&crypto, buf, len);
        dump_bytes(stdout, buf, len);
#else
        Slice e = slice_build_from_ptr_len((const char*) data[j].eptr, data[j].elen);
        buffer_clear(&b);
        buffer_append_slice(&b, e);
        uint32_t len = e.len;

        len = crypto_decrypt_cbc(&crypto, (uint8_t*) b.ptr, len);
        ok(len == data[j].dlen, "Got %d bytes with CBC Blowfish decryption for %s", len, data[j].label);
        ok(memcmp(b.ptr, data[j].dptr, data[j].dlen) == 0, "Got correct data with CBC Blowfish decryption for %s", data[j].label);

        len = crypto_encrypt_cbc(&crypto, (uint8_t*) b.ptr, len);
        ok(len == data[j].elen, "Got %d bytes with CBC Blowfish encryption for %s", len, data[j].label);
        ok(memcmp(b.ptr, data[j].eptr, data[j].elen) == 0, "Got correct data with CBC Blowfish encryption for %s", data[j].label);
#endif
    }
    buffer_destroy(&b);
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_crypto();

    done_testing();
}
