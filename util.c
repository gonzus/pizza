#include <ctype.h>
#include "util.h"

static void dump_line(FILE* fp, int row, const char* byte, int white, const char* text) {
    fprintf(fp, "%06x | %s%*s | %-16s |\n", row, byte, white, "", text);
}

void dump_bytes(FILE* fp, const unsigned char* bdat, int blen) {
    fprintf(fp, "bytes at %p, len %u\n", bdat, blen);
    char byte[16*3+1];
    int bpos = 0;
    char text[16+1];
    int dpos = 0;
    int row = 0;
    int col = 0;
    for (int j = 0; j < blen; ++j) {
        unsigned char uc = (unsigned char) bdat[j];
        unsigned int ui = (unsigned int) uc;
        bpos += sprintf(byte + bpos, "%s%02x", col ? " " : "", ui);
        dpos += sprintf(text + dpos, "%c", uc <= 0x7f && isprint(uc) ? uc : '.');
        col++;
        if (col == 16) {
            dump_line(fp, row, byte, 0, text);
            col = bpos = dpos = 0;
            row += 16;
        }
    }
    if (dpos > 0) {
        dump_line(fp, row, byte, (16-col)*3, text);
    }
}
