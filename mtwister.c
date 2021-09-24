#include <time.h>
#include "mtwister.h"

/*
 * This will be the step in the twist() loop
 */
#define STEP(state, tgt, src, nxt, za) \
    do { \
        uint32_t y = (state[tgt] & MASK_UPPER) | (state[nxt] & MASK_LOWER); \
        state[tgt] = state[src] ^ (y >> 1) ^ za[y & 1]; \
    } while (0)

/*
 * Define MT19937 constants (32-bit RNG)
 */
const uint32_t D = 0xffffffffU;
const uint32_t N = MTWISTER_STATE;
const uint32_t M = 397;
const uint32_t R = 31;
const uint32_t A = 0x9908b0df;
const uint32_t F = 1812433253;
const uint32_t U = 11;
const uint32_t Q = R - 1;

const uint32_t S = 7;
const uint32_t B = 0x9d2c5680;

const uint32_t T = 15;
const uint32_t C = 0xefc60000;

const uint32_t L = 18;

const uint32_t MASK_LOWER = (1U << R) - 1;
const uint32_t MASK_UPPER = (1U << R);

// Use either a 0 or the A value
static uint32_t zero_or_A[] = { 0, A };

// Initialize with a given numeric seed.
void mtwister_build_from_seed(MTwister* mt, const uint32_t seed) {
    mt->state[0] = seed;
    for (uint32_t j = 1; j < N; ++j) {
        mt->state[j] = (F * (mt->state[j - 1] ^ (mt->state[j - 1] >> Q)) + j);
    }
    mt->index = N;
}

// Initialize with a numeric key in an array.
void mtwister_build_from_key(MTwister* mt, const uint32_t key[], uint32_t len) {
    // I suspect Takuji Nishimura was born on this date...
    mtwister_build_from_seed(mt, 19650218U);
    uint32_t i = 1;
    uint32_t j = 0;
    uint32_t k = 0;
    for (k = N > len ? N : len; k; --k) {
        mt->state[i] = (mt->state[i] ^ ((mt->state[i - 1] ^ (mt->state[i - 1] >> Q)) * 1664525U)) + key[j] + j; // non linear
        mt->state[i] &= D; // for WORDSIZE > 32 machines
        ++i;
        ++j;
        if (i >= N) {
            mt->state[0] = mt->state[N - 1];
            i = 1;
        }
        if (j >= len) {
            j = 0;
        }
    }
    for (k = N - 1; k; --k) {
        mt->state[i] = (mt->state[i] ^ ((mt->state[i - 1] ^ (mt->state[i - 1] >> Q)) * 1566083941U)) - i; // non linear
        mt->state[i] &= D; // for WORDSIZE > 32 machines
        ++i;
        if (i >= N) {
            mt->state[0] = mt->state[N - 1];
            i = 1;
        }
    }

    mt->state[0] = 0x80000000U; // MSB is 1; assuring non-zero initial array
    mt->index = N;
}

// Initialize with a "random" seed.
void mtwister_build_from_random_seed(MTwister* mt) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    mtwister_build_from_seed(mt, ts.tv_nsec);
}

static void twist(MTwister* mt) {
    // This is just a loop in [0, N), but unrolled to avoid arithmetic % N
    for (uint32_t j = 0; j < N - M; ++j) {
        STEP(mt->state, j, j + M, j + 1, zero_or_A);
    }
    for (uint32_t j = N - M; j < N - 1; ++j) {
        STEP(mt->state, j, j + M - N, j + 1, zero_or_A);
    }
    for (uint32_t j = N - 1; j < N; ++j) {
        STEP(mt->state, j, j + M - N, 0, zero_or_A);
    }
    mt->index = 0;
}

// Generates a uint32_t random number on interval [0, 2^32 - 1]
uint32_t mtwister_generate_u32(MTwister* mt) {
    if (mt->index >= N) {
        twist(mt);
    }

    uint32_t y = mt->state[mt->index++];

    // Tempering
    y ^= (y >> U);
    y ^= (y << S) & B;
    y ^= (y << T) & C;
    y ^= (y >> L);

    return y;
}

// Generates a uint32_t random number on interval [0, 2^31 - 1]
uint32_t mtwister_generate_u31(MTwister* mt) {
    uint32_t y = mtwister_generate_u32(mt);
    return y >> 1;
}

// Generates a double random number on closed-closed interval [0, 1]
double mtwister_generate_double_CC(MTwister* mt) {
    uint32_t y = mtwister_generate_u32(mt);
    return (double) y * (1.0 / 4294967295.0); // divided by 2^32 - 1
}

// Generates a double random number on closed-open interval [0, 1)
double mtwister_generate_double_CO(MTwister* mt) {
    uint32_t y = mtwister_generate_u32(mt);
    return (double) y * (1.0 / 4294967296.0); // divided by 2^32
}

// Generates a double random number on open-open interval (0, 1)
double mtwister_generate_double_OO(MTwister* mt) {
    uint32_t y = mtwister_generate_u32(mt);
    return ((double)y + 0.5) * (1.0 / 4294967296.0); // divided by 2^32
}
