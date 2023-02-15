#ifndef REGEX_H_
#define REGEX_H_

#include "slice.h"

enum RegexError {
  RX_OK,
  RX_OOM,
  RX_INPUT_TOO_SHORT,
  RX_UNRECOGNIZED_CHARACTER,
  RX_UNKNOWN_ESCAPE,
  RX_UNKNOWN_POSIX_CHARCLASS,
  RX_UNMATCHED_PARENTHESIS,
  RX_UNMATCHED_BRACE,
  RX_UNMATCHED_BRACKET,
  RX_STAR_WITHOUT_OPERAND,
  RX_PLUS_WITHOUT_OPERAND,
  RX_QUESTION_WITHOUT_OPERAND,
  RX_LOHI_WITHOUT_OPERAND,
  RX_LOHI_HI_SMALLER_THAN_LO,
  RX_LOHI_BOTH_ZERO,
  RX_LAST,
};

// A compiled NFA (or an NFA that is in the process of being compiled) is an
// instance of the following object.
typedef struct Regex {
    Slice sIn;                 // Regular expression text
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

const char* regex_error(int err);

#endif
