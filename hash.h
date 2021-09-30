#ifndef HASH_H_
#define HASH_H_

/*
 * Useful hash functions.
 */

#include <stdint.h>

/*
 * http://www.cse.yorku.ca/~oz/hash.html
 */
uint32_t hash_djb2(const char* str, uint32_t len);

/*
 * https://fuchsia.googlesource.com/third_party/murmurhash.c/+/refs/heads/main
 */
uint32_t hash_murmur3(const char* str, uint32_t len, uint32_t seed);

/*
 * https://www.reedbeta.com/blog/quick-and-easy-gpu-random-numbers-in-d3d11/
 */
uint32_t hash_wang(uint32_t input);

/*
 * https://www.reedbeta.com/blog/hash-functions-for-gpu-rendering/
 */
uint32_t hash_pcg(uint32_t v);

#endif
