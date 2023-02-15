/*
 * Borrowed from
 * https://github.com/mackyle/sqlite/blob/c52e76d367a3b963a5e5d86834310b2843b83344/ext/misc/regexp.c
 */

/*
 * 2012-11-13
 *
 * The author disclaims copyright to this source code.  In place of
 * a legal notice, here is a blessing:
 *
 *    May you do good and not evil.
 *    May you find forgiveness for yourself and forgive others.
 *    May you share freely, never taking more than you give.
 *
 * -----------------------------------------------------------------
 *
 * The code in this file implements a compact but reasonably
 * efficient regular-expression matcher for posix extended regular
 * expressions against UTF8 text.
 *
 *  The following regular expression syntax is supported:
 *
 *     X*      zero or more occurrences of X
 *     X+      one or more occurrences of X
 *     X?      zero or one occurrences of X
 *     X{p,q}  between p and q occurrences of X
 *     (X)     match X
 *     X|Y     X or Y
 *     ^X      X occurring at the beginning of the string
 *     X$      X occurring at the end of the string
 *     .       Match any single character
 *     \c      Character c where c is one of \{}()[]|*+?.
 *     \c      C-language escapes for c in afnrtv.  ex: \t or \n
 *     \uXXXX  Where XXXX is exactly 4 hex digits, unicode value XXXX
 *     \xXX    Where XX is exactly 2 hex digits, unicode value XX
 *     [abc]   Any single character from the set abc
 *     [^abc]  Any single character not in the set abc
 *     [a-z]   Any single character in the range a-z
 *     [^a-z]  Any single character not in the range a-z
 *     \b      Word boundary
 *     \w      Word character.  [A-Za-z0-9_]
 *     \W      Non-word character
 *     \d      Digit
 *     \D      Non-digit
 *     \s      Whitespace character
 *     \S      Non-whitespace character
 *
 * A nondeterministic finite automaton (NFA) is used for matching, so the
 * performance is bounded by O(N*M) where N is the size of the regular
 * expression and M is the size of the input string.  The matcher never
 * exhibits exponential behavior.  Note that the X{p,q} operator expands
 * to p copies of X following by q-p copies of X? and that the size of the
 * regular expression in the O(N*M) performance bound is computed after
 * this expansion.
 */
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "regex.h"

#define SLICE_MOVE(s, l) \
  do { \
    (s).ptr += l; \
    (s).len -= l; \
  } while (0)

#define SLICE_LEN_GT(s, l)        ((s).len >  (l))
#define SLICE_LEN_GE(s, l)        ((s).len >= (l))
#define SLICE_LEN_LT(s, l)        ((s).len <  (l))
#define SLICE_LEN_LE(s, l)        ((s).len <= (l))

#define SLICE_ADVANCE(s, l)       SLICE_MOVE(s, +l)
#define SLICE_RETRACT(s, l)       SLICE_MOVE(s, -l)
#define SLICE_FINISHED(s)         SLICE_LEN_LE(s, 0)

#define sqlite3_malloc(s)         malloc(s)
#define sqlite3_malloc64(s)       malloc(s)
#define sqlite3_realloc64(p, s)   realloc(p, s)
#define sqlite3_free(p)           free(p)

#define CHAR_IN(c, l, r) ((c) >= (l) && (c) <= (r))

/* The end-of-input character */
#define RE_EOF            0    /* End of input */

/* The NFA is implemented as sequence of opcodes taken from the following
 * set.  Each opcode has a single integer argument.
 */
#define RE_OP_MATCH       1    /* Match the one character in the argument */
#define RE_OP_ANY         2    /* Match any one character.  (Implements ".") */
#define RE_OP_ANYSTAR     3    /* Special optimized version of .* */
#define RE_OP_FORK        4    /* Continue to both next and opcode at iArg */
#define RE_OP_GOTO        5    /* Jump to opcode at iArg */
#define RE_OP_ACCEPT      6    /* Halt and indicate a successful match */
#define RE_OP_CC_INC      7    /* Beginning of a [...] character class */
#define RE_OP_CC_EXC      8    /* Beginning of a [^...] character class */
#define RE_OP_CC_VALUE    9    /* Single value in a character class */
#define RE_OP_CC_RANGE   10    /* Range of values in a character class */
#define RE_OP_WORD       11    /* Perl word character [A-Za-z0-9_] */
#define RE_OP_NOTWORD    12    /* Not a perl word character */
#define RE_OP_DIGIT      13    /* digit:  [0-9] */
#define RE_OP_NOTDIGIT   14    /* Not a digit */
#define RE_OP_SPACE      15    /* space:  [ \t\n\r\v\f] */
#define RE_OP_NOTSPACE   16    /* Not a digit */
#define RE_OP_BOUNDARY   17    /* Boundary between word and non-word */

/* Each opcode is a "state" in the NFA */
typedef unsigned short ReStateNumber;

/* Because this is an NFA and not a DFA, multiple states can be active at
 * once.  An instance of the following object records all active states in
 * the NFA.  The implementation is optimized for the common case where the
 * number of actives states is small.
 */
typedef struct ReStateSet {
  unsigned nState;            /* Number of current states */
  ReStateNumber *aState;      /* Current states */
} ReStateSet;

void regex_build(Regex* rx) {
  memset(rx, 0, sizeof(Regex));
}

/* Free and reclaim all the memory used by a previously compiled
 * regular expression.  Applications should invoke this routine once
 * for every call to re_compile() to avoid memory leaks.
 */
static void regex_reset(Regex* rx) {
  if (!rx) return;
  if (rx->aOp) {
    sqlite3_free(rx->aOp);
    rx->aOp = 0;
  }
  if (rx->aArg) {
    sqlite3_free(rx->aArg);
    rx->aArg = 0;
  }
  memset(rx, 0, sizeof(Regex));
}

void regex_destroy(Regex* rx) {
  regex_reset(rx);
  memset(rx, 0, sizeof(Regex));
}

/* Add a state to the given state set if it is not already there */
static void re_add_state(ReStateSet *pSet, int newState) {
  for (unsigned i = 0; i < pSet->nState; ++i) {
    if (pSet->aState[i] == newState) return;
  }
  pSet->aState[pSet->nState++] = (ReStateNumber) newState;
}

/* Extract the next unicode character from *pzIn and return it.  Advance
 * *pzIn to the first byte past the end of the character returned.  To
 * be clear:  this routine converts utf8 to unicode.  This routine is
 * optimized for the common case where the next character is a single byte.
 */
static unsigned re_next_char(Slice *p) {
  if (SLICE_FINISHED(*p)) return 0;

  unsigned j = 0;
  unsigned c = p->ptr[j++];
  if (c >= 0x80) {
    if ((c & 0xe0) == 0xc0 && SLICE_LEN_GT(*p, j+0)
      && (p->ptr[j+0] & 0xc0) == 0x80) {
      c = (c & 0x1f) << 6 | (p->ptr[j+0] & 0x3f);
      j += 1;
      if (c < 0x80) c = 0xfffd;
    } else if ((c & 0xf0) == 0xe0 && SLICE_LEN_GT(*p, j+1)
      && (p->ptr[j+0] & 0xc0) == 0x80 && (p->ptr[j+1] & 0xc0) == 0x80) {
      c = (c & 0x0f) << 12 | ((p->ptr[j+0] & 0x3f) << 6) | (p->ptr[j+1] & 0x3f);
      j += 2;
      if (c <= 0x7ff || (c >= 0xd800 && c <= 0xdfff)) c = 0xfffd;
    } else if ((c & 0xf8) == 0xf0 && SLICE_LEN_GT(*p, j+2)
      && (p->ptr[j+0] & 0xc0) == 0x80 && (p->ptr[j+1] & 0xc0) == 0x80 && (p->ptr[j+2] & 0xc0) == 0x80) {
      c = (c & 0x07) << 18 | ((p->ptr[j+0] & 0x3f) << 12) | ((p->ptr[j+1] & 0x3f) << 6) | (p->ptr[j+2] & 0x3f);
      j += 3;
      if (c <= 0xffff || c > 0x10ffff) c = 0xfffd;
    } else {
      c = 0xfffd;
    }
  }
  SLICE_ADVANCE(*p, j);
  return c;
}

static unsigned re_next_char_nocase(Slice *p) {
  unsigned c = re_next_char(p);
  if (CHAR_IN(c, 'A', 'Z')) c += 'a' - 'A';
  return c;
}

/* Return true if c is a perl "word" character:  [A-Za-z0-9_] */
static int re_word_char(int c) {
  return CHAR_IN(c, '0', '9') || CHAR_IN(c, 'a', 'z') || CHAR_IN(c, 'A', 'Z') || c == '_';
}

/* Return true if c is a "digit" character:  [0-9] */
static int re_digit_char(int c) {
  return CHAR_IN(c, '0', '9');
}

/* Return true if c is a perl "space" character:  [ \t\r\n\v\f] */
static int re_space_char(int c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
}

/* Run a compiled regular expression on the zero-terminated input
 * string zIn[].  Return true on a match and false if there is no match.
 */
int regex_match(Regex* rx, Slice text, int* matched) {
  *matched = 0;
  /* Look for the initial prefix match, if there is one. */
  if (rx->nInit) {
    unsigned j = 0;
    unsigned char x = rx->zInit[0];
    while (SLICE_LEN_GE(text, j + rx->nInit)
      && (text.ptr[j] != x || strncmp(text.ptr + j, (const char*) rx->zInit, rx->nInit) != 0)
    ) {
        ++j;
      }
    if (SLICE_LEN_LT(text, j + rx->nInit)) return 0;
  }

  ReStateSet aStateSet[2];
  ReStateNumber aSpace[100];
  ReStateNumber *pToFree = 0;
  if (rx->nState <= (sizeof(aSpace) / (sizeof(aSpace[0]) * 2))) {
    aStateSet[0].aState = aSpace;
  } else {
    pToFree = (ReStateNumber*) sqlite3_malloc64(sizeof(ReStateNumber) * 2 * rx->nState);
    if (pToFree == 0) {
      rx->zErr = "out of memory 2";
      return 201;
    }
    aStateSet[0].aState = pToFree;
  }
  aStateSet[1].aState = &aStateSet[0].aState[rx->nState];
  ReStateSet *pNext = &aStateSet[1];
  pNext->nState = 0;
  re_add_state(pNext, 0);

  int rc = 0;
  unsigned int iSwap = 0;
  for (int c = RE_EOF + 1; !rc && c != RE_EOF && pNext->nState > 0; ) {
    int cPrev = c;
    c = rx->next(&text);
    ReStateSet *pThis = pNext;
    pNext = &aStateSet[iSwap];
    pNext->nState = 0;
    iSwap = 1 - iSwap;
    for (unsigned int i = 0; !rc && i < pThis->nState; ++i) {
      int x = pThis->aState[i];
      switch (rx->aOp[x]) {
        case RE_OP_MATCH:
          if (rx->aArg[x] == c) re_add_state(pNext, x + 1);
          break;

        case RE_OP_ANY:
          if (c != 0) re_add_state(pNext, x + 1);
          break;

        case RE_OP_WORD:
          if (re_word_char(c)) re_add_state(pNext, x + 1);
          break;

        case RE_OP_NOTWORD:
          if (!re_word_char(c) && c != 0) re_add_state(pNext, x + 1);
          break;

        case RE_OP_DIGIT:
          if (re_digit_char(c)) re_add_state(pNext, x + 1);
          break;

        case RE_OP_NOTDIGIT:
          if (!re_digit_char(c) && c != 0) re_add_state(pNext, x + 1);
          break;

        case RE_OP_SPACE:
          if (re_space_char(c)) re_add_state(pNext, x + 1);
          break;

        case RE_OP_NOTSPACE:
          if (!re_space_char(c) && c != 0) re_add_state(pNext, x + 1);
          break;

        case RE_OP_BOUNDARY:
          if (re_word_char(c) != re_word_char(cPrev)) re_add_state(pThis, x + 1);
          break;

        case RE_OP_ANYSTAR:
          re_add_state(pNext, x);
          re_add_state(pThis, x + 1);
          break;

        case RE_OP_FORK:
          re_add_state(pThis, x + rx->aArg[x]);
          re_add_state(pThis, x + 1);
          break;

        case RE_OP_GOTO:
          re_add_state(pThis, x + rx->aArg[x]);
          break;

        case RE_OP_ACCEPT:
          rc = 1;
          break;

        case RE_OP_CC_EXC:
          if (c == 0) break;
        /* FALL-THROUGH */
        case RE_OP_CC_INC:
          {
            int n = rx->aArg[x];
            int hit = 0;
            for (int j = 1; j > 0 && j < n; ++j) {
              if (rx->aOp[x + j] == RE_OP_CC_VALUE) {
                if (rx->aArg[x + j] == c) {
                  hit = 1;
                  j = -1;
                }
              } else {
                if (rx->aArg[x + j] <= c && rx->aArg[x + j + 1] >= c) {
                  hit = 1;
                  j = -1;
                } else {
                  ++j;
                }
              }
            }
            if (rx->aOp[x] == RE_OP_CC_EXC) hit = !hit;
            if (hit) re_add_state(pNext, x + n);
            break;
          }
      }
    }
  }
  for (unsigned int i = 0; !rc && i < pNext->nState; ++i) {
    if (rx->aOp[pNext->aState[i]] == RE_OP_ACCEPT) {
      rc = 1;
      break;
    }
  }

  sqlite3_free((void*) pToFree);
  *matched = rc;
  return 0;
}

/* Resize the opcode and argument arrays for an RE under construction.
*/
static int regex_resize(Regex* rx, int N) {
  char *aOp = (char*) sqlite3_realloc64(rx->aOp, N * sizeof(rx->aOp[0]));
  if (aOp == 0) return 1;
  rx->aOp = aOp;
  int *aArg = (int*) sqlite3_realloc64(rx->aArg, N * sizeof(rx->aArg[0]));
  if (aArg == 0) return 1;
  rx->aArg = aArg;
  rx->nAlloc = N;
  return 0;
}

/* Insert a new opcode and argument into an RE under construction.  The
 * insertion point is just prior to existing opcode iBefore.
 */
static int re_insert(Regex *p, int iBefore, int op, int arg) {
  if (p->nAlloc <= p->nState && regex_resize(p, p->nAlloc * 2)) return 0;
  for (int i = p->nState; i > iBefore; --i) {
    p->aOp[i] = p->aOp[i - 1];
    p->aArg[i] = p->aArg[i - 1];
  }
  ++p->nState;
  p->aOp[iBefore] = (char) op;
  p->aArg[iBefore] = arg;
  return iBefore;
}

/* Append a new opcode and argument to the end of the RE under construction.
*/
static int re_append(Regex *p, int op, int arg) {
  return re_insert(p, p->nState, op, arg);
}

/* Make a copy of N opcodes starting at iStart onto the end of the RE
 * under construction.
 */
static void re_copy(Regex *p, int iStart, int N) {
  if (p->nState + N >= p->nAlloc && regex_resize(p, p->nAlloc * 2 + N)) return;
  memcpy(&p->aOp[p->nState], &p->aOp[iStart], N * sizeof(p->aOp[0]));
  memcpy(&p->aArg[p->nState], &p->aArg[iStart], N * sizeof(p->aArg[0]));
  p->nState += N;
}

/* Return true if c is a hexadecimal digit character:  [0-9a-fA-F]
 * If c is a hex digit, also set *pV = (*pV)*16 + valueof(c).  If
 * c is not a hex digit *pV is unchanged.
 */
static int re_hex(int c, int *pV) {
  if (CHAR_IN(c, '0', '9')) {
    c -= '0';
  } else if (CHAR_IN(c, 'a', 'f')) {
    c -= 'a' - 10;
  } else if (CHAR_IN(c, 'A', 'F')) {
    c -= 'A' - 10;
  } else {
    return 0;
  }
  *pV = *pV * 16 + (c & 0xff);
  return 1;
}

/* A backslash character has been seen, read the next character and
 * return its interpretation.
 */
static unsigned re_esc_char(Regex *p) {
  static const char zEsc[] = "afnrtv\\()*.+?[$^{|}]";
  static const char zTrans[] = "\a\f\n\r\t\v";
  if (SLICE_FINISHED(p->sIn)) return 0;

  unsigned j = 0;
  char c = p->sIn.ptr[j];
  int v = 0;
  if (c == 'u' && SLICE_LEN_GE(p->sIn, 5)) {
    const unsigned char *zIn = (const unsigned char*) (p->sIn.ptr + j);
    if (re_hex(zIn[1],&v)
      && re_hex(zIn[2],&v)
      && re_hex(zIn[3],&v)
      && re_hex(zIn[4],&v)
    ) {
      SLICE_ADVANCE(p->sIn, 5);
      return v;
    }
  }
  if (c == 'x' && SLICE_LEN_GE(p->sIn, 3)) {
    const unsigned char *zIn = (const unsigned char*) (p->sIn.ptr + j);
    if (re_hex(zIn[1],&v)
      && re_hex(zIn[2],&v)
    ) {
      SLICE_ADVANCE(p->sIn, 3);
      return v;
    }
  }

  int i = 0;
  for (i = 0; zEsc[i] && zEsc[i] != c; ++i) {}
  if (zEsc[i]) {
    if (i < 6) c = zTrans[i];
    ++j;
  } else {
    p->zErr = "unknown \\ escape";
  }
  SLICE_ADVANCE(p->sIn, j);
  return c;
}

/* Forward declaration */
static int re_subcompile_string(Regex*);

/* Peek at the next byte of input */
static unsigned char rePeek(Regex *p) {
  return SLICE_FINISHED(p->sIn) ? 0 : p->sIn.ptr[0];
}

/* Compile RE text into a sequence of opcodes.  Continue up to the
 * first unmatched ")" character, then return.  If an error is found,
 * return a pointer to the error message string.
 */
static int re_subcompile_re(Regex *p) {
  int err = 0;
  int iStart = p->nState;
  err = re_subcompile_string(p);
  if (err) return err;

  while (rePeek(p) == '|') {
    int iEnd = p->nState;
    re_insert(p, iStart, RE_OP_FORK, iEnd + 2 - iStart);
    int iGoto = re_append(p, RE_OP_GOTO, 0);
    SLICE_ADVANCE(p->sIn, 1);
    err = re_subcompile_string(p);
    if (err) return err;
    p->aArg[iGoto] = p->nState - iGoto;
  }
  return 0;
}

/* Compile an element of regular expression text (anything that can be
 * an operand to the "|" operator).  Return NULL on success or a pointer
 * to the error message if there is a problem.
 */
static int re_subcompile_string(Regex *p) {
  int iPrev = -1;
  unsigned c = 0;
  while ((c = p->next(&p->sIn)) != 0) {
    int iStart = p->nState;
    switch (c) {
      case '|':
      case '$':
      case ')':
        SLICE_RETRACT(p->sIn, 1);
        return 0;

      case '(':
        {
          int err = re_subcompile_re(p);
          if (err) return err;
          if (rePeek(p) != ')') {
            p->zErr = "unmatched '('";
            return 101;
          }
          SLICE_ADVANCE(p->sIn, 1);
          break;
        }

      case '.':
        if (rePeek(p) == '*') {
          re_append(p, RE_OP_ANYSTAR, 0);
          SLICE_ADVANCE(p->sIn, 1);
        } else {
          re_append(p, RE_OP_ANY, 0);
        }
        break;

      case '*':
        if (iPrev < 0) {
          p->zErr = "'*' without operand";
          return 102;
        }
        re_insert(p, iPrev, RE_OP_GOTO, p->nState - iPrev + 1);
        re_append(p, RE_OP_FORK, iPrev - p->nState + 1);
        break;

      case '+':
        if (iPrev < 0) {
          p->zErr = "'+' without operand";
          return 103;
        }
        re_append(p, RE_OP_FORK, iPrev - p->nState);
        break;

      case '?':
        if (iPrev < 0) {
          p->zErr = "'?' without operand";
          return 104;
        }
        re_insert(p, iPrev, RE_OP_FORK, p->nState - iPrev + 1);
        break;

      case '{':
        {
          if (iPrev < 0) {
            p->zErr = "'{m,n}' without operand";
            return 105;
          }

          int m = 0;
          while (1) {
            c = rePeek(p);
            if (!CHAR_IN(c, '0', '9')) break;
            m = m * 10 + c - '0';
            SLICE_ADVANCE(p->sIn, 1);
          }
          int n = m;
          if (c == ',') {
            SLICE_ADVANCE(p->sIn, 1);
            n = 0;
            while ((c = rePeek(p)) >= '0' && c <= '9') {
              n = n * 10 + c - '0';
              SLICE_ADVANCE(p->sIn, 1);
            }
          }
          if (c != '}') {
            p->zErr = "unmatched '{'";
            return 106;
          }
          if (n > 0 && n < m) {
            p->zErr = "n less than m in '{m,n}'";
            return 107;
          }

          SLICE_ADVANCE(p->sIn, 1);
          int sz = p->nState - iPrev;
          if (m == 0) {
            if (n == 0) {
              p->zErr = "both m and n are zero in '{m,n}'";
              return 108;
            }
            re_insert(p, iPrev, RE_OP_FORK, sz + 1);
            --n;
          } else {
            for (int j = 1; j < m; ++j) re_copy(p, iPrev, sz);
          }
          for (int j = m; j < n; ++j) {
            re_append(p, RE_OP_FORK, sz + 1);
            re_copy(p, iPrev, sz);
          }
          if (n == 0 && m > 0) {
            re_append(p, RE_OP_FORK, -sz);
          }
          break;
        }

      case '[':
        {
          int iFirst = p->nState;
          if (rePeek(p) == '^') {
            re_append(p, RE_OP_CC_EXC, 0);
            SLICE_ADVANCE(p->sIn, 1);
          } else {
            re_append(p, RE_OP_CC_INC, 0);
          }
          while ((c = p->next(&p->sIn)) != 0) {
            if (c == '[' && rePeek(p) == ':') {
              p->zErr = "POSIX character classes not supported";
              return 109;
            }
            if (c == '\\') c = re_esc_char(p);
            if (rePeek(p) == '-') {
              re_append(p, RE_OP_CC_RANGE, c);
              SLICE_ADVANCE(p->sIn, 1);
              c = p->next(&p->sIn);
              if (c == '\\') c = re_esc_char(p);
              re_append(p, RE_OP_CC_RANGE, c);
            } else {
              re_append(p, RE_OP_CC_VALUE, c);
            }
            if (rePeek(p) == ']') {
              SLICE_ADVANCE(p->sIn, 1);
              break;
            }
          }
          if (c == 0) {
            p->zErr = "unclosed '['";
            return 110;
          }
          p->aArg[iFirst] = p->nState - iFirst;
          break;
        }

      case '\\':
        {
          int specialOp = 0;
          switch (rePeek(p)) {
            case 'b': specialOp = RE_OP_BOUNDARY;   break;
            case 'd': specialOp = RE_OP_DIGIT;      break;
            case 'D': specialOp = RE_OP_NOTDIGIT;   break;
            case 's': specialOp = RE_OP_SPACE;      break;
            case 'S': specialOp = RE_OP_NOTSPACE;   break;
            case 'w': specialOp = RE_OP_WORD;       break;
            case 'W': specialOp = RE_OP_NOTWORD;    break;
          }
          if (specialOp) {
            SLICE_ADVANCE(p->sIn, 1);
            re_append(p, specialOp, 0);
          } else {
            c = re_esc_char(p);
            re_append(p, RE_OP_MATCH, c);
          }
          break;
        }

      default:
        re_append(p, RE_OP_MATCH, c);
        break;

    }
    iPrev = iStart;
  }
  return 0;
}

/*
 * Compile a textual regular expression in zIn[] into a compiled regular
 * expression suitable for use by regex_match() and return a pointer to the
 * compiled regular expression in *ppRe.  Return NULL on success or an
 * error message if something goes wrong.
 */
int regex_compile(Regex* rx, Slice source, int case_insensitive) {
  regex_reset(rx);
  rx->next = case_insensitive ? re_next_char_nocase : re_next_char;
  if (regex_resize(rx, 30)) {
    regex_reset(rx);
    rx->zErr = "out of memory 1";
    return 201;
  }
  unsigned j = 0;
  if (source.ptr[j] == '^') {
    ++j;
  } else {
    re_append(rx, RE_OP_ANYSTAR, 0);
  }
  rx->sIn = slice_from_memory(source.ptr + j, source.len - j);
  int err = re_subcompile_re(rx);
  if (err) {
    regex_reset(rx);
    return err;
  }
  if (rePeek(rx) == '$' && SLICE_LEN_LE(rx->sIn, 1)) {
    re_append(rx, RE_OP_MATCH, RE_EOF);
    re_append(rx, RE_OP_ACCEPT, 0);
  } else if (SLICE_FINISHED(rx->sIn)) {
    re_append(rx, RE_OP_ACCEPT, 0);
  } else {
    regex_reset(rx);
    rx->zErr = "unrecognized character";
    return 202;
  }

  /*
   * The following is a performance optimization.  If the regex begins with
   * ".*" (if the input regex lacks an initial "^") and afterwards there are
   * one or more matching characters, enter those matching characters into
   * zInit[].  The regex_match() routine can then search ahead in the input
   * string looking for the initial match without having to run the whole
   * regex engine over the string.  Do not worry able trying to match
   * unicode characters beyond plane 0 - those are very rare and this is
   * just an optimization.
   */
  if (rx->aOp[0] == RE_OP_ANYSTAR && !case_insensitive) {
    j = 0;
    for (int i = 1; j < sizeof(rx->zInit) - 2 && rx->aOp[i] == RE_OP_MATCH; ++i) {
      unsigned x = rx->aArg[i];
      if (x <= 127) {
        rx->zInit[j++] = (unsigned char) x;
      } else if (x <= 0xfff) {
        rx->zInit[j++] = (unsigned char) (0xc0 | (x >> 6));
        rx->zInit[j++] = 0x80 | (x & 0x3f);
      } else if (x <= 0xffff) {
        rx->zInit[j++] = (unsigned char) (0xd0 | (x >> 12));
        rx->zInit[j++] = 0x80 | ((x >> 6) & 0x3f);
        rx->zInit[j++] = 0x80 | (x & 0x3f);
      } else {
        break;
      }
    }
    if (j > 0 && rx->zInit[j - 1] == 0) --j;
    rx->nInit = j;
  }
  if (rx->zErr) {
    LOG_INFO("Found error [%s]", rx->zErr);
    return 999;
  }
  return 0;
}
