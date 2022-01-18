#ifndef CRYPTO_H_
#define CRYPTO_H_

/*
 * CBC decryption -- using Blowfish and PKCS#7 padding
 * https://en.wikipedia.org/wiki/Block_cipher_mode_of_operation
 * https://en.wikipedia.org/wiki/Blowfish_(cipher)
 * https://en.wikipedia.org/wiki/Padding_%28cryptography%29
 *
 * These work IN-PLACE, so the original contents of ptr are overwritten.
 * NOTE: encryption might use up MORE than len bytes; up to CRYPTO_BLOCK_SIZE
 * more bytes, in fact, due to padding.
 */

#include "blowfish.h"
#include "slice.h"

#define CRYPTO_KEY_SIZE 56
#define CRYPTO_BLOCK_SIZE 8

typedef struct Crypto {
    Blowfish bf;
    char key[2*CRYPTO_KEY_SIZE]; // with some extra room
    char iv[CRYPTO_BLOCK_SIZE];
} Crypto;

void crypto_init(Crypto* crypto, Slice passphrase, Slice iv);

// decrypts in-place len bytes in ptr;
// never uses more than len bytes;
// returns length of decrypted data, which might be smaller than len;
uint32_t crypto_decrypt_cbc(Crypto* crypto, uint8_t* ptr, uint32_t len);

// encrypts in-place len bytes in ptr;
// it WILL use between 1 and CRYPTO_BLOCK_SIZE more bytes than len;
// returns length of encrypted data, which WILL be larger than len;
uint32_t crypto_encrypt_cbc(Crypto* crypto, uint8_t* ptr, uint32_t len);

#endif
