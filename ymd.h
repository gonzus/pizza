#ifndef YMD_H_
#define YMD_H_

/*
 * Date-related functions.
 */

// Doh.
#define YMD_DAYS_PER_WEEK    7
#define YMD_MONTHS_PER_YEAR 12

// Numeric values for days of the week.
#define YMD_DOW_MON 0
#define YMD_DOW_TUE 1
#define YMD_DOW_WED 2
#define YMD_DOW_THU 3
#define YMD_DOW_FRI 4
#define YMD_DOW_SAT 5
#define YMD_DOW_SUN 6

// Numeric values for months of the year.
#define YMD_MOY_JAN  1
#define YMD_MOY_FEB  2
#define YMD_MOY_MAR  3
#define YMD_MOY_APR  4
#define YMD_MOY_MAY  5
#define YMD_MOY_JUN  6
#define YMD_MOY_JUL  7
#define YMD_MOY_AUG  8
#define YMD_MOY_SEP  9
#define YMD_MOY_OCT 10
#define YMD_MOY_NOV 11
#define YMD_MOY_DEC 12

// Macro to determine the day of the week given a Julian day
// number.
#define ymd_dow(j) ((j) % YMD_DAYS_PER_WEEK)

// Return a string with the day name for a given day of the week.
// d: 0 (Mon) to 6 (Sun).
const char* ymd_day_name(int d);

// Return a string with the month name for a given month of the year.
// m: 1 (Jan) to 12 (Dec)
const char* ymd_month_name(int m);

// Given a date encoded as YYYYMMDD, break it into its components.
// Return a reencoded value of the form YYYYMMDD.
// Return respective components if pointers are passed in.
int ymd_decode(int ymd, int* y, int* m, int* d);

// Given a y/m/d, return the date as an encoded value.
// Return a value of the form YYYYMMDD.
int ymd_encode(int y, int m, int d);

// Get today's date.
// Return a value of the form YYYYMMDD.
// Return respective components if pointers are passed in.
int ymd_today(int* y, int* m, int* d);

// Return zero if year is not leap, non-zero otherwise.
int ymd_is_leap_year(int y);

// Return the number of days in a given month of a year.
// This changes due to leap years.
int ymd_days_in_month(int y, int m);

// Return the Julian day number for a given date.
int ymd_to_julian(int y, int m, int d);

// Get the date for a given Julian day number.
// Return a value of the form YYYYMMDD.
// Return respective components if pointers are passed in.
int ymd_from_julian(int j, int* y, int* m, int* d);

// Get the date of Easter for a given year.
// Return a value of the form YYYYMMDD.
// Return respective components if pointers are passed in.
int ymd_easter(int y, int* m, int* d);

#endif
