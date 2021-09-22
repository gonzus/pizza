#include <stdio.h>
#include <stdlib.h>
#include "mtwister.h"

int main(int argc, char* argv[]) {
    MTwister mt;
    uint32_t key[] = { 0x123, 0x234, 0x345, 0x456 };
    mtwister_build_from_key(&mt, key, 4);
    int top = 1000;
    printf("%d outputs of genrand_int32()\n", top);
    for (int j = 0; j < top; ++j) {
        uint32_t r = mtwister_generate_u32(&mt);
        printf("%10u ", r);
        if (j % 5 == 4) {
            printf("\n");
        }
    }
    printf("\n");

    printf("%d outputs of genrand_real2()\n", top);
    for (int j = 0; j < top; ++j) {
        double r = mtwister_generate_double_CO(&mt);
        printf("%10.8f ", r);
        if (j % 5 == 4) {
            printf("\n");
        }
    }
    printf("\n");

    return 0;
}
