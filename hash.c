#include <stdio.h>
#include "hash.h"

uint32_t hash_djb2(const char* str, uint32_t len) {
    uint32_t h = 5381;
    for (uint32_t j = 0; j < len; ++j) {
        char c = str[j];
        h = ((h << 5) + h) + c; // h * 33 + c
    }
    return h;
}

uint32_t hash_murmur3(const char* str, uint32_t len, uint32_t seed) {
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;
    const uint32_t r1 = 15;
    const uint32_t r2 = 13;
    const uint32_t m = 5;
    const uint32_t n = 0xe6546b64;
    const uint32_t l = len / 4; // chunk length
    const uint8_t* d = (uint8_t*) str; // 32 bit extract from `str'
    const uint32_t* chunks = (const uint32_t*) (d + l * 4); // body
    const uint8_t* tail = (const uint8_t*) (d + l * 4); // last 8 byte chunk of `str'

    uint32_t h = seed;
    uint32_t k = 0;
    // for each 4 byte chunk of `str'
    for (int j = -l; j != 0; ++j) {
        // next 4 byte chunk of `str'
        k = chunks[j];
        // encode next 4 byte chunk of `str'
        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;
        // append to hash
        h ^= k;
        h = (h << r2) | (h >> (32 - r2));
        h = h * m + n;
    }
    k = 0;
    // remainder
    switch (len & 3) { // `len % 4'
        case 3:
            k ^= (tail[2] << 16);
            /* FALLTHROUGH */
        case 2:
            k ^= (tail[1] << 8);
            /* FALLTHROUGH */
        case 1:
            k ^= tail[0];
            k *= c1;
            k = (k << r1) | (k >> (32 - r1));
            k *= c2;
            h ^= k;
            /* FALLTHROUGH */
    }
    h ^= len;
    h ^= (h >> 16);
    h *= 0x85ebca6b;
    h ^= (h >> 13);
    h *= 0xc2b2ae35;
    h ^= (h >> 16);
    return h;
}

uint32_t hash_wang(uint32_t input) {
    input = (input ^ 61) ^ (input >> 16);
    input *= 9;
    input = input ^ (input >> 4);
    input = input * 0x27d4eb2d;
    input = input ^ (input >> 15);
    return input;
}

uint32_t hash_pcg(uint32_t input)
{
    uint32_t state = input * 747796405u + 2891336453u;
    uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}
