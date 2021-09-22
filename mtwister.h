#ifndef MTWISTER_H_
#define MTWISTER_H_

#include <stdint.h>

#define MTWISTER_STATE 624

typedef struct MTwister {
    uint32_t state[MTWISTER_STATE];
    uint16_t index;
} MTwister;

// Initialize with a given numeric seed.
void mtwister_build_from_seed(MTwister* mt, const uint32_t seed);

// Initialize with a numeric key in an array.
void mtwister_build_from_key(MTwister* mt, const uint32_t key[], int len);

// Generates a uint32_t random number on interval [0, 2^32 - 1]
uint32_t mtwister_generate_u32(MTwister* mt);

// Generates a uint32_t random number on interval [0, 2^31 - 1]
uint32_t mtwister_generate_u31(MTwister* mt);

// Generates a double random number on closed-closed interval [0, 1]
double mtwister_generate_double_CC(MTwister* mt);

// Generates a double random number on closed-open interval [0, 1)
double mtwister_generate_double_CO(MTwister* mt);

// Generates a double random number on open-open interval (0, 1)
double mtwister_generate_double_OO(MTwister* mt);

#endif
