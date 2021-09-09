#include <time.h>
#include "hms.h"

static int hms_valid_hms(int hms, int h, int m, int s) {
    return (hms == 0 &&
            h >= 0 && h < HMS_HOURS_PER_DAY &&
            m >= 0 && m < HMS_MINUTES_PER_HOUR &&
            s >= 0 && s < HMS_SECONDS_PER_MINUTE);
}

int hms_decode(int hms, int* h, int* m, int* s) {
    int ss = hms % 100;
    hms /= 100;
    int mm = hms % 100;
    hms /= 100;
    int hh = hms % 100;
    hms /= 100;

    if (!hms_valid_hms(hms, hh, mm, ss)) {
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
    if (!hms_valid_hms(0, h, m, s)) {
        h = m = s = 0;
    }
    return (h * 100 + m) * 100 + s;
}

int hms_now(int* h, int* m, int* s) {
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

int hms_to_elapsed(int h, int m, int s) {
    return (h * HMS_MINUTES_PER_HOUR + m) * HMS_SECONDS_PER_MINUTE + s;
}

int hms_from_elapsed(int e, int* h, int* m, int* s) {
    int ss = e % HMS_SECONDS_PER_MINUTE;
    e /= HMS_SECONDS_PER_MINUTE;
    int mm = e % HMS_MINUTES_PER_HOUR;
    e /= HMS_MINUTES_PER_HOUR;
    int hh = e % HMS_HOURS_PER_DAY;
    e /= HMS_HOURS_PER_DAY;

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
