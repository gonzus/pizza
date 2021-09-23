#include <stdio.h>
#include "mtwister.h"

int main(int argc, char* argv[]) {
    MTwister mt;
    mtwister_build_from_random_seed(&mt);
    int top = 10;
    for (int j = 0; j < top; ++j) {
        uint32_t r = mtwister_generate_u32(&mt);
        printf("%10u\n", r);
    }

    return 0;
}
