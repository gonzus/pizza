#ifndef BLOWFISH_H_
#define BLOWFISH_H_

/*
 * blowfish encryption and decryption.
 */

#include <stdint.h>

#define BLOWFISH_NUM_SUBKEYS  18
#define BLOWFISH_NUM_S_BOXES  4
#define BLOWFISH_NUM_ENTRIES  256

typedef struct Blowfish {
    uint32_t PA[BLOWFISH_NUM_SUBKEYS];
    uint32_t SB[BLOWFISH_NUM_S_BOXES][BLOWFISH_NUM_ENTRIES];
} Blowfish;

// Initialize a Blowfish context to encrypt / decrypt using key with len bytes.
void blowfish_init(Blowfish* bf, const char* key, uint32_t len);

// Encrypt a block of len bytes where the data is stored in Little Endian order.
void blowfish_encrypt_LE(Blowfish* bf, uint8_t* ptr, uint32_t len);

// Encrypt a block of len bytes where the data is stored in Big Endian order.
void blowfish_encrypt_BE(Blowfish* bf, uint8_t* ptr, uint32_t len);

// Decrypt a block of len bytes where the data is stored in Little Endian order.
void blowfish_decrypt_LE(Blowfish* bf, uint8_t* ptr, uint32_t len);

// Decrypt a block of len bytes where the data is stored in Big Endian order.
void blowfish_decrypt_BE(Blowfish* bf, uint8_t* ptr, uint32_t len);

#endif
