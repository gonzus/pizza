#ifndef REGEX_H_
#define REGEX_H_

/*
 * An input string read one character at a time.
 */
typedef struct ReInput ReInput;
struct ReInput {
    const unsigned char *z;  /* All text */
    int i;                   /* Next byte to read */
    int mx;                  /* EOF when i >= mx */
};

/*
 * A compiled NFA (or an NFA that is in the process of being compiled) is an
 * instance of the following object.
 */
typedef struct ReCompiled ReCompiled;
struct ReCompiled {
    ReInput sIn;                /* Regular expression text */
    const char *zErr;           /* Error message to return */
    char *aOp;                  /* Operators for the virtual machine */
    int *aArg;                  /* Arguments to each operator */
    unsigned (*xNextChar)(ReInput*);  /* Next character function */
    unsigned char zInit[12];    /* Initial text to match */
    int nInit;                  /* Number of characters in zInit */
    unsigned nState;            /* Number of entries in aOp[] and aArg[] */
    unsigned nAlloc;            /* Slots allocated for aOp[] and aArg[] */
};

// TODO: decide on the definite API for this
// Hopefully use Slice and Buffer, and stop allocating memory manually?
const char *re_compile(ReCompiled **ppRe, const char *zIn, int noCase);
int re_match(ReCompiled *pRe, const unsigned char *zIn, int nIn);
void re_free(ReCompiled *pRe);

#endif
