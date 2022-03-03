#include <string.h>
#include "pizza/blowfish.h"
#include "pizza/md5.h"
#include "pizza/crypto.h"

#define DECRYPT(pos, iv, orig, last) \
    do { \
        memcpy(iv, orig, CRYPTO_BLOCK_SIZE); \
        uint8_t block[CRYPTO_BLOCK_SIZE]; \
        for (uint32_t k = 0; k < CRYPTO_BLOCK_SIZE; ++k) { \
            uint32_t p = pos + k; \
            orig[k] = block[k] = p < len ? ptr[p] : 0; \
        } \
        blowfish_decrypt_BE(&crypto->bf, block, CRYPTO_BLOCK_SIZE); \
        for (uint32_t k = 0; k < CRYPTO_BLOCK_SIZE; ++k) { \
            uint32_t p = pos + k; \
            last = ptr[p] = iv[k] ^ block[k]; \
        } \
    } while (0)

#define ENCRYPT(pos, iv, block_size, pad) \
    do { \
        uint8_t block[CRYPTO_BLOCK_SIZE]; \
        for (uint32_t k = 0; k < CRYPTO_BLOCK_SIZE; ++k) { \
            uint32_t p = pos + k; \
            block[k] = iv[k] ^ (pad ? pad : p < len ? ptr[p] : block_size); \
        } \
        blowfish_encrypt_BE(&crypto->bf, block, CRYPTO_BLOCK_SIZE); \
        for (uint32_t k = 0; k < CRYPTO_BLOCK_SIZE; ++k) { \
            uint32_t p = pos + k; \
            ptr[p] = block[k]; \
        } \
        memcpy(iv, block, CRYPTO_BLOCK_SIZE); \
    } while (0)

static uint32_t generate_key(Slice passphrase, char* key);

void crypto_init(Crypto* crypto, Slice passphrase, Slice iv) {
    assert(iv.len == CRYPTO_BLOCK_SIZE);

    memset(crypto, 0, sizeof(Crypto));
    generate_key(passphrase, crypto->key);
    memcpy(crypto->iv, iv.ptr, CRYPTO_BLOCK_SIZE);
    blowfish_init(&crypto->bf, crypto->key, CRYPTO_KEY_SIZE);
}

uint32_t crypto_decrypt_cbc(Crypto* crypto, uint8_t* ptr, uint32_t len) {
    // orig contains the block before decryption;
    // for the first iteration, it contains the IV
    uint8_t orig[CRYPTO_BLOCK_SIZE];
    memcpy(orig, crypto->iv, CRYPTO_BLOCK_SIZE);

    uint8_t last = 0;
    uint8_t iv[CRYPTO_BLOCK_SIZE];
    for (uint32_t pos = 0; pos < len; pos += CRYPTO_BLOCK_SIZE) {
        DECRYPT(pos, iv, orig, last);
    }

    // return corrected length, discounting padding
    len -= last;
    return len;
}

uint32_t crypto_encrypt_cbc(Crypto* crypto, uint8_t* ptr, uint32_t len) {
    uint8_t iv[CRYPTO_BLOCK_SIZE];
    memcpy(iv, crypto->iv, CRYPTO_BLOCK_SIZE);

    uint8_t block_size = 0;
    for (uint32_t pos = 0; pos < len; pos += CRYPTO_BLOCK_SIZE) {
        block_size = len - pos;
        ENCRYPT(pos, iv, CRYPTO_BLOCK_SIZE - block_size, 0);
    }

    if (block_size == CRYPTO_BLOCK_SIZE) {
        // need extra block
        ENCRYPT(len, iv, 0, block_size);
        len += CRYPTO_BLOCK_SIZE;
    }

    // return corrected length, accounting for padding
    len += CRYPTO_BLOCK_SIZE - block_size;
    return len;
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
        source = slice_from_memory(key, len);
    }
    return len;
}
