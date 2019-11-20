#include <time.h>
#include "date.h"

const char* date_day_name(int d)
{
    static const char* day_name[] = {
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday",
        "Sunday",
    };

    // day numbers are 0-based
    return day_name[d % DATE_DAYS_PER_WEEK];
}

const char* date_month_name(int m)
{
    static const char* month_name[] = {
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
    return month_name[m % DATE_MONTHS_PER_YEAR];
}

int date_encode(int y, int m, int d)
{
    return (y * 100 + m) * 100 + d;
}

int date_decode(int date, int* y, int* m, int* d)
{
    int dd = date % 100;
    date /= 100;
    int mm = date % 100;
    date /= 100;
    int yy = date % 10000;

    if (y) {
        *y = yy;
    }
    if (m) {
        *m = mm;
    }
    if (d) {
        *d = dd;
    }

    return date_encode(yy, mm, dd);
}

int date_today(int* y, int* m, int* d)
{
    time_t now = time(0);
    struct tm local;
    localtime_r(&now, &local);

    int yy = local.tm_year + 1900;
    int mm = local.tm_mon  + 1;
    int dd = local.tm_mday;

    if (y) {
        *y = yy;
    }
    if (m) {
        *m = mm;
    }
    if (d) {
        *d = dd;
    }

    return date_encode(yy, mm, dd);
}

int date_is_leap_year(int y)
{
    return ((y % 4) == 0) && ((y % 100) != 0 || (y % 400) == 0);
}

int date_days_in_month(int y, int m)
{
    switch (m) {
        case DATE_MOY_JAN:
        case DATE_MOY_MAR:
        case DATE_MOY_MAY:
        case DATE_MOY_JUL:
        case DATE_MOY_AUG:
        case DATE_MOY_OCT:
        case DATE_MOY_DEC:
            return 31;

        case DATE_MOY_APR:
        case DATE_MOY_JUN:
        case DATE_MOY_SEP:
        case DATE_MOY_NOV:
            return 30;

        case DATE_MOY_FEB:
            return date_is_leap_year(y) ? 29 : 28;

        default:
            return 0;
    }
}

// See https://www.hermetic.ch/cal_stud/jdn.htm for details.
int date_ymd_to_julian(int y, int m, int d)
{
    int p = (m - 14) / 12;
    int j = (1461 * (y + 4800 + p)) / 4 +
            (367 * (m - 2 - 12 * p)) / 12 -
            (3 * ((y + 4900 + p) / 100)) / 4 +
            (d - 32075);
    return j;
}

// See https://www.hermetic.ch/cal_stud/jdn.htm for details.
int date_julian_to_ymd(int j, int* y, int* m, int* d)
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
    return date_encode(yy, mm, dd);
}

// This algorithm to compute Easter was invented by Karl Friedrich Gauss.
// So yes, karl RULES.
// See http://aa.usno.navy.mil/faq/docs/easter.php for details.
int date_easter(int y, int* m, int* d)
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
    return date_encode(y, mm, dd);
}
