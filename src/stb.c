/*
 * This file is used to create the implementation of any stb modules used by
 * the library.
 */

// STB sprintf() and related functions.
#define STB_SPRINTF_IMPLEMENTATION

// On my M1 laptop I am seeing these errors from USAN, hence the following define.
//
// stb_sprintf.h:427:17: runtime error: store to misaligned address 0x00016b906523
//                       for type 'unsigned int', which requires 4 byte alignment
#define STB_SPRINTF_NOUNALIGNED
#include "stb_sprintf.h"
