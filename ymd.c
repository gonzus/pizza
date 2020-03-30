#include <time.h>
#include "ymd.h"

const char* ymd_day_name(int d)
{
    static const char* day_name[YMD_DAYS_PER_WEEK] = {
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday",
        "Sunday",
    };

    // day numbers are 0-based
    int dd = d % YMD_DAYS_PER_WEEK;
    if (dd != d) return "";
    return day_name[dd];
}

const char* ymd_month_name(int m)
{
    static const char* month_name[YMD_MONTHS_PER_YEAR] = {
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

    // month numbers are 1-based
    --m;
    int mm = m % YMD_MONTHS_PER_YEAR;
    if (mm != m) return "";
    return month_name[mm];
}

int ymd_decode(int ymd, int* y, int* m, int* d)
{
    int dd = ymd % 100;
    ymd /= 100;
    int mm = ymd % 100;
    ymd /= 100;
    int yy = ymd % 10000;
    ymd /= 10000;

    if (ymd != 0 ||
        mm < 1 || mm > 12 ||
        dd < 1 || dd > ymd_days_in_month(yy, mm)) {
        yy = mm = dd = 0;
    }

    if (y) {
        *y = yy;
    }
    if (m) {
        *m = mm;
    }
    if (d) {
        *d = dd;
    }

    return ymd_encode(yy, mm, dd);
}

int ymd_encode(int y, int m, int d) {
    if (m < 1 || m > 12 ||
        d < 1 || d > ymd_days_in_month(y, m)) {
        y = m = d = 0;
    }
    return (y * 100 + m) * 100 + d;
}

int ymd_today(int* y, int* m, int* d)
{
    time_t seconds = time(0);
    struct tm* local = localtime(&seconds);

    int yy = local->tm_year + 1900;
    int mm = local->tm_mon  + 1;
    int dd = local->tm_mday;

    if (y) {
        *y = yy;
    }
    if (m) {
        *m = mm;
    }
    if (d) {
        *d = dd;
    }

    return ymd_encode(yy, mm, dd);
}

// See https://en.wikipedia.org/wiki/Leap_year#Algorithm for details.
int ymd_is_leap_year(int y)
{
    return ((y % 4) == 0) && ((y % 100) != 0 || (y % 400) == 0);
}

int ymd_days_in_month(int y, int m)
{
    switch (m) {
        case YMD_MOY_JAN:
        case YMD_MOY_MAR:
        case YMD_MOY_MAY:
        case YMD_MOY_JUL:
        case YMD_MOY_AUG:
        case YMD_MOY_OCT:
        case YMD_MOY_DEC:
            return 31;

        case YMD_MOY_APR:
        case YMD_MOY_JUN:
        case YMD_MOY_SEP:
        case YMD_MOY_NOV:
            return 30;

        case YMD_MOY_FEB:
            return ymd_is_leap_year(y) ? 29 : 28;

        default:
            return 0;
    }
}

// See https://www.hermetic.ch/cal_stud/jdn.htm for details.
int ymd_to_julian(int y, int m, int d)
{
    int p = (m - 14) / 12;
    int j = (1461 * (y + 4800 + p)) / 4 +
            (367 * (m - 2 - 12 * p)) / 12 -
            (3 * ((y + 4900 + p) / 100)) / 4 +
            (d - 32075);
    return j;
}

// See https://www.hermetic.ch/cal_stud/jdn.htm for details.
int ymd_from_julian(int j, int* y, int* m, int* d)
{
    int l = j + 68569;
    int n = (4 * l) / 146097;
    l -= (146097 * n + 3) / 4;
    int i = (4000 * (l + 1)) / 1461001;
    l -= (1461 * i) / 4 - 31;
    int h = (80 * l) / 2447;
    int k = h / 11;
    int dd = l - (2447 * h) / 80;
    int mm = h + 2 - (12 * k);
    int yy = 100 * (n - 49) + i + k;

    if (y) {
        *y = yy;
    }
    if (m) {
        *m = mm;
    }
    if (d) {
        *d = dd;
    }

    return ymd_encode(yy, mm, dd);
}

// This algorithm to compute Easter was invented by Karl Friedrich Gauss.
// So yes, karl RULES.
// See http://aa.usno.navy.mil/faq/docs/easter.php for details.
int ymd_easter(int y, int* m, int* d)
{
    int k = y / 100;
    int a = k / 4;
    int r = y % 19;
    int l = (k - 17) / 25;

    int R = (k - a - (k - l) / 3 + 19 * r + 15) % 30;
    int U = R / 28;
    int L = R - U * (1 - U * (29 / (R + 1)) * ((21 - r) / 11));
    int E = (y + y / 4 + L + 2 - k + a) % 7;
    int S = L - E;

    int mm = 3 + (S + 40) / 44;
    int dd = S + 28 - 31 * (mm / 4);

    if (m) {
        *m = mm;
    }
    if (d) {
        *d = dd;
    }

    return ymd_encode(y, mm, dd);
}
