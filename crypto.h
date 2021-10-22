#ifndef CRYPTO_H_
#define CRYPTO_H_

/*
 * CBC decryption -- using Blowfish.
 */

#include "blowfish.h"
#include "slice.h"

#define CRYPTO_KEY_SIZE 56
#define CRYPTO_BLOCK_SIZE 8

typedef struct Crypto {
    Blowfish bf;
    char key[2*CRYPTO_KEY_SIZE]; // with some padding
    char iv[CRYPTO_BLOCK_SIZE];
} Crypto;

void crypto_init(Crypto* crypto, Slice passphrase, Slice iv);
void crypto_decrypt_cbc(Crypto* crypto, uint8_t* ptr, uint32_t len);

#endif
