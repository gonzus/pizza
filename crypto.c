#include <assert.h>
#include <string.h>
#include "blowfish.h"
#include "md5.h"
#include "crypto.h"

static uint32_t generate_key(Slice passphrase, char* key);

void crypto_init(Crypto* crypto, Slice passphrase, Slice iv) {
    assert(iv.len == CRYPTO_BLOCK_SIZE);

    memset(crypto, 0, sizeof(Crypto));
    generate_key(passphrase, crypto->key);
    memcpy(crypto->iv, iv.ptr, CRYPTO_BLOCK_SIZE);
    blowfish_init(&crypto->bf, crypto->key, CRYPTO_KEY_SIZE);
}

void crypto_decrypt_cbc(Crypto* crypto, uint8_t* ptr, uint32_t len) {
    // orig contains the block before encryption;
    // for the first iteration, it contains the IV
    uint8_t orig[CRYPTO_BLOCK_SIZE];
    memcpy(orig, crypto->iv, CRYPTO_BLOCK_SIZE);

    uint8_t iv[CRYPTO_BLOCK_SIZE];
    for (uint32_t j = 0; j < len; j += CRYPTO_BLOCK_SIZE) {
        // use current orig as the IV for this iteration
        memcpy(iv, orig, CRYPTO_BLOCK_SIZE);

        // copy next block into fresh memory
        uint8_t block[CRYPTO_BLOCK_SIZE];
        for (uint32_t k = 0; k < CRYPTO_BLOCK_SIZE; ++k) {
            uint32_t p = j + k;
            orig[k] = block[k] = p < len ? ptr[p] : 0; // TODO: padding?
        }

        // encrypt block
        blowfish_decrypt_BE(&crypto->bf, block, CRYPTO_BLOCK_SIZE);

        // copy IV XOR block back
        for (uint32_t k = 0; k < CRYPTO_BLOCK_SIZE; ++k) {
            uint32_t p = j + k;
            if (p >= len) continue;
            ptr[p] = iv[k] ^ block[k];
        }
    }
}

static uint32_t generate_key(Slice passphrase, char* key) {
    uint32_t len = 0;
    Slice source = passphrase;
    while (len < CRYPTO_KEY_SIZE) {
        MD5 md5;
        md5_reset(&md5);
        md5_update(&md5, source);
        md5_finalize(&md5);
        memcpy(key + len, md5.digest, MD5_DIGEST_LEN);
        len += MD5_DIGEST_LEN;
        source = slice_build_from_ptr_len(key, len);
    }
    return len;
}
