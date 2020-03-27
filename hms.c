#include <time.h>
#include "hms.h"

int hms_decode(int hms, int* h, int* m, int* s)
{
    int ss = hms % 100;
    hms /= 100;
    int mm = hms % 100;
    hms /= 100;
    int hh = hms % 100;
    hms /= 100;

    if (hms != 0 ||
        hh < 0 || hh > 23 ||
        mm < 0 || mm > 59 ||
        ss < 0 || ss > 59) {
        hh = mm = ss = 0;
    }

    if (h) {
        *h = hh;
    }
    if (m) {
        *m = mm;
    }
    if (s) {
        *s = ss;
    }

    return hms_encode(hh, mm, ss);
}

int hms_encode(int h, int m, int s) {
    if (h < 0 || h > 23 ||
        m < 0 || m > 59 ||
        s < 0 || s > 59) {
        h = m = s = 0;
    }
    return (h * 100 + m) * 100 + s;
}

int hms_now(int* h, int* m, int* s)
{
    time_t seconds = time(0);
    struct tm* local = localtime(&seconds);

    int hh = local->tm_hour;
    int mm = local->tm_min;
    int ss = local->tm_sec;

    if (h) {
        *h = hh;
    }
    if (m) {
        *m = mm;
    }
    if (s) {
        *s = ss;
    }

    return hms_encode(hh, mm, ss);
}

int hms_to_elapsed(int h, int m, int s)
{
    return (h * HMS_MINUTES_PER_HOUR + m) * HMS_SECONDS_PER_MINUTE + s;
}

int hms_from_elapsed(int e, int* h, int* m, int* s)
{
    int ss = e % HMS_SECONDS_PER_MINUTE;
    e /= HMS_SECONDS_PER_MINUTE;
    int mm = e % HMS_MINUTES_PER_HOUR;
    e /= HMS_MINUTES_PER_HOUR;
    int hh = e % HMS_HOURS_PER_DAY;
    e /= HMS_HOURS_PER_DAY;
    // TODO: check?
    // TODO: check in date too?

    if (h) {
        *h = hh;
    }
    if (m) {
        *m = mm;
    }
    if (s) {
        *s = ss;
    }
    return hms_encode(hh, mm, ss);
}
