#ifndef REGEX_H_
#define REGEX_H_

#include "slice.h"

// A compiled NFA (or an NFA that is in the process of being compiled) is an
// instance of the following object.
typedef struct Regex {
    Slice sIn;                 // Regular expression text
    const char *zErr;          // Most recent error message
    char *aOp;                 // Operators for the virtual machine
    int *aArg;                 // Arguments to each operator
    unsigned (*next)(Slice*);  // Next character function
    unsigned char zInit[12];   // Initial text to match
    int nInit;                 // Number of characters in zInit
    unsigned nState;           // Number of entries in aOp[] and aArg[]
    unsigned nAlloc;           // Slots allocated for aOp[] and aArg[]
} Regex;

void regex_build(Regex* rx);
void regex_destroy(Regex* rx);
int regex_compile(Regex* rx, Slice source, int case_insensitive);
int regex_match(Regex* rx, Slice text, int* matched);

#endif
