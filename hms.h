#ifndef HMS_H_
#define HMS_H_

/*
 * Time-related functions.
 * HMS == Hour Minute Second
 */

// Doh.
#define HMS_SECONDS_PER_MINUTE    60
#define HMS_MINUTES_PER_HOUR      60
#define HMS_HOURS_PER_DAY         24

// Given a time encoded as HHMMSS, break it into its components.
// Return a reencoded value of the form HHMMSS.
// Return respective components if pointers are passed in.
int hms_decode(int hms, int* h, int* m, int* s);

// Given an h/m/s, return the time as an encoded value.
// Return a value of the form HHMMSS.
int hms_encode(int h, int m, int s);

// Get current time.
// Return a value of the form HHMMSS.
// Return respective components if pointers are passed in.
int hms_now(int* h, int* m, int* s);

// Return the elapsed time since 00:00:00 for a given time.
int hms_to_elapsed(int h, int m, int s);

// Get the time for a given elapsed time since 00:00:00.
// Return a value of the form HHMMSS.
// Return respective components if pointers are passed in.
int hms_from_elapsed(int e, int* h, int* m, int* s);

#endif
