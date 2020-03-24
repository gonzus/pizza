#include <time.h>
#include <tap.h>
#include "date.h"

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

static void test_date_day_name(void) {
    for (int d = 0; d < ALEN(day_names); ++d) {
        const char* e = day_names[d];
        const char* n = date_day_name(d);
        is(n, e, "date_day_name %s OK", e);
    }
    is(date_day_name(99), "", "date_date_name 99 EMPTY");
}

static void test_date_month_name(void) {
    for (int m = 1; m < ALEN(month_names); ++m) {
        const char* e = month_names[m];
        const char* n = date_month_name(m);
        is(n, e, "date_month_name %s OK", e);
    }
    is(date_month_name(99), "", "date_month_name 99 EMPTY");
}

static void test_date_encode_decode(void) {
    int Y = 1990;
    int M = 11;
    int D = 20;

    int y = Y;
    int m = M;
    int d = D;
    int dec = 0;

    int enc = date_encode(y, m, d);
    cmp_ok(enc, "==", 19901120, "date_encode");

    y = m = d = 0;
    dec = date_decode(enc, &y, &m, &d);
    cmp_ok(dec, "==", 19901120, "date_decode YMD");
    cmp_ok(y, "==", Y, "date_decode Y");
    cmp_ok(m, "==", M, "date_decode M");
    cmp_ok(d, "==", D, "date_decode D");

    y = m = d = 0;
    dec = date_decode(enc, &y, &m, 0);
    cmp_ok(dec, "==", 19901120, "date_decode YM");
    cmp_ok(y, "==", Y, "date_decode Y");
    cmp_ok(m, "==", M, "date_decode M");
    cmp_ok(d, "==", 0, "date_decode D");

    y = m = d = 0;
    dec = date_decode(enc, &y, 0, 0);
    cmp_ok(dec, "==", 19901120, "date_decode Y");
    cmp_ok(y, "==", Y, "date_decode Y");
    cmp_ok(m, "==", 0, "date_decode M");
    cmp_ok(d, "==", 0, "date_decode D");

    y = m = d = 0;
    dec = date_decode(enc, 0, 0, 0);
    cmp_ok(dec, "==", 19901120, "date_decode");
    cmp_ok(y, "==", 0, "date_decode Y");
    cmp_ok(m, "==", 0, "date_decode M");
    cmp_ok(d, "==", 0, "date_decode D");
}

static void test_date_today(void) {
    int y = 0;
    int m = 0;
    int d = 0;
    date_today(&y, &m, &d);

    time_t seconds = time(0);
    struct tm* local = localtime(&seconds);
    int Y = local->tm_year + 1900;
    int M = local->tm_mon  + 1;
    int D = local->tm_mday;

    cmp_ok(y, "==", Y, "date_today Y");
    cmp_ok(m, "==", M, "date_today M");
    cmp_ok(d, "==", D, "date_today D");
}

static void test_date_is_leap_year(void) {
    for (int j = 0; j < ALEN(year_info); ++j) {
        int y = year_info[j].year;
        int l = year_info[j].leap;
        int i = date_is_leap_year(y);
        cmp_ok(!!i, "==", !!l, "date_is_leap_year(%d) OK", y);
    }
}

static void test_date_days_in_month(void) {
    for (int j = 0; j < ALEN(year_info); ++j) {
        int y = year_info[j].year;
        int l = year_info[j].leap;
        for (int m = 1; m <= 12; ++m) {
            int d = date_days_in_month(y, m);
            int e = days_per_month[m];
            if (l && m == 2) {
                e = 29;
            }
            cmp_ok(d, "==", e, "date_days_in_month(%d, %d) OK", y, m);
        }
    }
}

static void test_date_easter(void) {
    for (int j = 0; j < ALEN(year_info); ++j) {
        int Y = year_info[j].year;
        int M = year_info[j].emon;
        int D = year_info[j].eday;
        int m = 0;
        int d = 0;
        date_easter(Y, &m, &d);
        cmp_ok(m*100+d, "==", M*100+D, "date_easter(%d) OK", Y);
    }
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_date_day_name();
    test_date_month_name();
    test_date_encode_decode();
    test_date_today();
    test_date_is_leap_year();
    test_date_days_in_month();
    test_date_easter();

    done_testing();
}
