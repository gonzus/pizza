#include <stdio.h>
#include "pizza.h"

int main(int argc, char* argv[]) {
    const char* name = "nico";
    int year = 2005;
    char tmp[100];

    slice s1 = slice_wrap_ptr(name);
    buffer* b = slice_to_buffer(s1);

    sprintf(tmp, " was born in %d", year);
    buffer_append_string(b, tmp);
    buffer_append_string(b, "!");
    slice s2 = buffer_to_slice(b);

    slice_to_string(s2, tmp);
    printf("[%s]\n", tmp);
    return 0;
}
