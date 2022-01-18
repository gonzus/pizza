#include <time.h>
#include <tap.h>
#include "pizza/ymd.h"

#define ALEN(a) (int) ((sizeof(a) / sizeof((a)[0])))

static const char* day_names[] = {
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    "Sunday",
};
static const char* month_names[] = { // 1-based
    "",
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December",
};
static int days_per_month[] = {
    0,31,28,31,30,31,30,31,31,30,31,30,31, // 1-based
};
static struct YearInfo {
    int year;
    int leap;
    int emon;
    int eday;
} year_info[] = {
    { 2020, 1, 4, 12 },
    { 2019, 0, 4, 21 },
    { 2000, 1, 4, 23 },
    { 2013, 0, 3, 31 },
    { 2012, 1, 4,  8 },
    { 1900, 0, 4, 15 },
};

static void test_ymd_day_name(void) {
    for (int d = 0; d < ALEN(day_names); ++d) {
        const char* e = day_names[d];
        const char* n = ymd_day_name(d);
        is(n, e, "ymd_day_name %s OK", e);
    }
    is(ymd_day_name(99), "", "ymd_day_name 99 EMPTY");
}

static void test_ymd_month_name(void) {
    for (int m = 1; m < ALEN(month_names); ++m) {
        const char* e = month_names[m];
        const char* n = ymd_month_name(m);
        is(n, e, "ymd_month_name %s OK", e);
    }
    is(ymd_month_name(99), "", "ymd_month_name 99 EMPTY");
}

static void test_ymd_encode_decode(void) {
    static struct {
        int y;
        int m;
        int d;
        int e;
    } data[] = {
        { 1990, 11, 20, 19901120 },
        { 2000,  2, 29, 20000229 },
        { 1990,  0, 20, 0 },
        { 1990, 13, 20, 0 },
        { 1990, 11,  0, 0 },
        { 1990,  2, 29, 0 },
        { 1990,  4, 31, 0 },
        { 1990,  6, 31, 0 },
        { 1990,  9, 31, 0 },
        { 1990, 11, 31, 0 },
    };

    for (int j = 0; j < ALEN(data); ++j) {
        int Y = data[j].y;
        int M = data[j].m;
        int D = data[j].d;
        int E = data[j].e;
        int y = Y;
        int m = M;
        int d = D;
        if (!E) {
            Y = M = D = 0;
        }

        int enc = ymd_encode(y, m, d);
        cmp_ok(enc, "==", E, "ymd_encode(%d, %d, %d) %s", y, m, d, E ? "OK" : "BAD");

        for (int k = 0; k < 4; ++k) {
            int YY = Y;
            int MM = M;
            int DD = D;
            int dec = 0;
            y = m = d = 0;
            switch (k) {
                case 0:
                    dec = ymd_decode(enc, &y, &m, &d);
                    break;
                case 1:
                    dec = ymd_decode(enc, &y, &m, 0);
                    DD = 0;
                    break;
                case 2:
                    dec = ymd_decode(enc, &y, 0, 0);
                    MM = 0;
                    DD = 0;
                    break;
                case 3:
                    dec = ymd_decode(enc, 0, 0, 0);
                    YY = 0;
                    MM = 0;
                    DD = 0;
                    break;
            }
            cmp_ok(dec, "==", E, "ymd_decode(%d) YMD %s", enc, E ? "OK" : "BAD");
            cmp_ok(y, "==", YY, "ymd_decode(%d) %c %s", enc, YY ? 'Y' : '0', E ? "OK" : "BAD");
            cmp_ok(m, "==", MM, "ymd_decode(%d) %c %s", enc, MM ? 'M' : '0', E ? "OK" : "BAD");
            cmp_ok(d, "==", DD, "ymd_decode(%d) %c %s", enc, DD ? 'D' : '0', E ? "OK" : "BAD");
        }
    }
}

static void test_ymd_today(void) {
    int y = 0;
    int m = 0;
    int d = 0;
    ymd_today(&y, &m, &d);

    time_t seconds = time(0);
    struct tm* local = localtime(&seconds);
    int Y = local->tm_year + 1900;
    int M = local->tm_mon  + 1;
    int D = local->tm_mday;

    cmp_ok(y, "==", Y, "ymd_today Y %d OK", Y);
    cmp_ok(m, "==", M, "ymd_today M %d OK", M);
    cmp_ok(d, "==", D, "ymd_today D %d OK", D);
}

static void test_ymd_is_leap_year(void) {
    for (int j = 0; j < ALEN(year_info); ++j) {
        int y = year_info[j].year;
        int l = year_info[j].leap;
        int i = ymd_is_leap_year(y);
        cmp_ok(!!i, "==", !!l, "ymd_is_leap_year(%d) %s OK", y, l ? "yes" : "no");
    }
}

static void test_ymd_days_in_month(void) {
    for (int j = 0; j < ALEN(year_info); ++j) {
        int y = year_info[j].year;
        int l = year_info[j].leap;
        for (int m = 1; m <= 12; ++m) {
            int d = ymd_days_in_month(y, m);
            int e = days_per_month[m];
            if (l && m == 2) {
                e = 29;
            }
            cmp_ok(d, "==", e, "ymd_days_in_month(%d, %d) %d OK", y, m, e);
        }
    }
}

static void test_ymd_easter(void) {
    for (int j = 0; j < ALEN(year_info); ++j) {
        int Y = year_info[j].year;
        int M = year_info[j].emon;
        int D = year_info[j].eday;
        int m = 0;
        int d = 0;
        ymd_easter(Y, &m, &d);
        cmp_ok(m*100+d, "==", M*100+D, "ymd_easter(%d) %d %d (%s) OK", Y, M, D, month_names[M]);
    }
}

static void test_ymd_julian(void) {
    static struct {
        int beg;
        int end;
        int delta;
    } data[] = {
        { 20200320, 20200327, 7 },
        { 19991231, 20000101, 1 },
        { 20080229, 20080301, 1 },
        { 20181231, 20190101, 1 },
        { 20190131, 20190201, 1 },
        { 20190228, 20190301, 1 },
        { 20190331, 20190401, 1 },
        { 20190430, 20190501, 1 },
        { 20190531, 20190601, 1 },
        { 20190630, 20190701, 1 },
        { 20190731, 20190801, 1 },
        { 20190831, 20190901, 1 },
        { 20190930, 20191001, 1 },
        { 20191031, 20191101, 1 },
        { 20191130, 20191201, 1 },
        { 20191231, 20200101, 1 },
    };
    for (int j = 0; j < ALEN(data); ++j) {
        int by, bm, bd;
        ymd_decode(data[j].beg, &by, &bm, &bd);
        int bj = ymd_to_julian(by, bm, bd);
        int benc = ymd_from_julian(bj, 0, 0, 0);
        cmp_ok(data[j].beg, "==", benc, "ymd_julian(%d) roundtrips OK", data[j].beg);

        int ey, em, ed;
        ymd_decode(data[j].end, &ey, &em, &ed);
        int ej = ymd_to_julian(ey, em, ed);
        int eenc = ymd_from_julian(ej, 0, 0, 0);
        cmp_ok(data[j].end, "==", eenc, "ymd_julian(%d) roundtrips OK", data[j].end);

        int d = ej - bj;
        cmp_ok(d, "==", data[j].delta, "ymd difference %d - %d = %d OK", data[j].end, data[j].beg, d);
    }
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_ymd_day_name();
    test_ymd_month_name();
    test_ymd_encode_decode();
    test_ymd_today();
    test_ymd_is_leap_year();
    test_ymd_days_in_month();
    test_ymd_easter();
    test_ymd_julian();

    done_testing();
}
