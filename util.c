#include <ctype.h>
#include "buffer.h"
#include "timer.h"
#include "util.h"

static struct {
    unsigned long min;
    const char* name;
} units[] = {
    { 7UL * 24UL * 60UL * 60UL * USECS_IN_A_SEC, "W"  },
    {       24UL * 60UL * 60UL * USECS_IN_A_SEC, "D"  },
    {              60UL * 60UL * USECS_IN_A_SEC, "h"  },
    {                     60UL * USECS_IN_A_SEC, "m"  },
    {                            USECS_IN_A_SEC, "s"  },
    {                           USECS_IN_A_MSEC, "ms" },
    { /* default, catch all */              0UL, "us" },
};

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

void format_detailed_us(unsigned long us, struct Buffer* b) {
    for (int j = 0; 1; ++j) {
        if (us < units[j].min) {
            continue;
        }
        unsigned long value = us;
        if (units[j].min == 0) {
            us = 0;
        } else {
            value /=  units[j].min;
            us -= value * units[j].min;
        }
        buffer_format_unsigned(b, value);
        buffer_append_string(b, units[j].name, -1);
        if (!us) {
            break;
        }
    }
}

void format_abbreviated_us(unsigned long us, struct Buffer* b) {
    for (int j = 0; 1; ++j) {
        if (us < units[j].min) {
            continue;
        }
        double value = us;
        if (units[j].min == 0) {
        } else {
            value /=  units[j].min;
        }
        buffer_format_print(b, "%.2f%s", value, units[j].name);
        break;
    }
}