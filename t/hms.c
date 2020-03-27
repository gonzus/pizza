#include <time.h>
#include <tap.h>
#include "hms.h"

#define ALEN(a) (int) ((sizeof(a) / sizeof((a)[0])))

static void test_hms_encode_decode(void) {
    static struct {
        int h;
        int m;
        int s;
        int e;
    } data[] = {
        { 17, 44, 20, 174420 },
        {  3,  2, 11,  30211 },
        { 24,  2, 11,  0 },
        {  3, 62, 11,  0 },
        {  3,  2, 71,  0 },
    };

    for (int j = 0; j < ALEN(data); ++j) {
        int H = data[j].h;
        int M = data[j].m;
        int S = data[j].s;
        int E = data[j].e;
        int h = H;
        int m = M;
        int s = S;
        if (!E) {
            H = M = S = 0;
        }

        int enc = hms_encode(h, m, s);
        cmp_ok(enc, "==", E, "hms_encode(%d, %d, %d) %s", h, m, s, E ? "OK" : "BAD");

        for (int k = 0; k < 4; ++k) {
            int HH = H;
            int MM = M;
            int SS = S;
            int dec = 0;
            h = m = s = 0;
            switch (k) {
                case 0:
                    dec = hms_decode(enc, &h, &m, &s);
                    break;
                case 1:
                    dec = hms_decode(enc, &h, &m, 0);
                    SS = 0;
                    break;
                case 2:
                    dec = hms_decode(enc, &h, 0, 0);
                    MM = 0;
                    SS = 0;
                    break;
                case 3:
                    dec = hms_decode(enc, 0, 0, 0);
                    HH = 0;
                    MM = 0;
                    SS = 0;
                    break;
            }
            cmp_ok(dec, "==", E, "hms_decode(%d) HMS %s", enc, E ? "OK" : "BAD");
            cmp_ok(h, "==", HH, "hms_decode(%d) %c %s", enc, HH ? 'H' : '0', E ? "OK" : "BAD");
            cmp_ok(m, "==", MM, "hms_decode(%d) %c %s", enc, MM ? 'M' : '0', E ? "OK" : "BAD");
            cmp_ok(s, "==", SS, "hms_decode(%d) %c %s", enc, SS ? 'S' : '0', E ? "OK" : "BAD");
        }
    }
}

static void test_hms_now(void) {
    int h = 0;
    int m = 0;
    int s = 0;
    hms_now(&h, &m, &s);

    time_t seconds = time(0);
    struct tm* local = localtime(&seconds);
    int H = local->tm_hour;
    int M = local->tm_min;
    int S = local->tm_sec;

    cmp_ok(h, "==", H, "hms_now H %d OK", H);
    cmp_ok(m, "==", M, "hms_now M %d OK", M);
    cmp_ok(s, "==", S, "hms_now S %d OK", S);
}

static void test_hms_elapsed(void) {
    static struct {
        int beg;
        int end;
        int delta;
    } data[] = {
        { 134355, 134401, 6 },
        {      0,      1, 1 },
        {  95959, 100102, 63 },
        { 135955, 140005, 10 },
        { 195958, 200002, 4 },
    };
    for (int j = 0; j < ALEN(data); ++j) {
        int bh, bm, bs;
        hms_decode(data[j].beg, &bh, &bm, &bs);
        int be = hms_to_elapsed(bh, bm, bs);
        int benc = hms_from_elapsed(be, 0, 0, 0);
        cmp_ok(data[j].beg, "==", benc, "hms elapsed (%d) roundtrips OK", data[j].beg);

        int eh, em, es;
        hms_decode(data[j].end, &eh, &em, &es);
        int ee = hms_to_elapsed(eh, em, es);
        int eenc = hms_from_elapsed(ee, 0, 0, 0);
        cmp_ok(data[j].end, "==", eenc, "hms elapsed (%d) roundtrips OK", data[j].end);

        int d = ee - be;
        cmp_ok(d, "==", data[j].delta, "hmd difference %d - %d = %d OK", data[j].end, data[j].beg, d);
    }
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_hms_encode_decode();
    test_hms_now();
    test_hms_elapsed();

    done_testing();
}
