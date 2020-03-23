#ifndef DATE_H_
#define DATE_H_

/*
 * Date-related functions.
 */

// Doh.
#define DATE_DAYS_PER_WEEK    7
#define DATE_MONTHS_PER_YEAR 12

// Numeric values for days of the week.
#define DATE_DOW_MON 0
#define DATE_DOW_TUE 1
#define DATE_DOW_WED 2
#define DATE_DOW_THU 3
#define DATE_DOW_FRI 4
#define DATE_DOW_SAT 5
#define DATE_DOW_SUN 6

// Numeric values for months of the year.
#define DATE_MOY_JAN  1
#define DATE_MOY_FEB  2
#define DATE_MOY_MAR  3
#define DATE_MOY_APR  4
#define DATE_MOY_MAY  5
#define DATE_MOY_JUN  6
#define DATE_MOY_JUL  7
#define DATE_MOY_AUG  8
#define DATE_MOY_SEP  9
#define DATE_MOY_OCT 10
#define DATE_MOY_NOV 11
#define DATE_MOY_DEC 12

// Given a y/m/d, return the date as an encoded value.
// Return a value of the form YYYYMMDD.
#define date_encode(y, m, d) (((y) * 100 + (m)) * 100 + (d))

// Macro to determine the day of the week given a Julian day
// number.
#define date_dow(j) ((j) % DATE_DAYS_PER_WEEK)

// Return a string with the day name for a given day of the week.
// d: 0 (Mon) to 6 (Sun).
const char* date_day_name(int d);

// Return a string with the month name for a given month of the year.
// m: 1 (Jan) to 12 (Dec)
const char* date_month_name(int m);

// Given a date encoded as YYYYMMDD, break it into its components.
// Return a reencoded value of the form YYYYMMDD.
// Return respective components if pointers are passed in.
int date_decode(int date, int* y, int* m, int* d);

// Get today's date.
// Return a value of the form YYYYMMDD.
// Return respective components if pointers are passed in.
int date_today(int* y, int* m, int* d);

// Return zero if year is not leap, non-zero otherwise.
int date_is_leap_year(int y);

// Return the number of days in a given month of a year.
// This changes due to leap years.
int date_days_in_month(int y, int m);

// Return the Julian day number for a given date.
int date_ymd_to_julian(int y, int m, int d);

// Get the date for a given Julian day number.
// Return a value of the form YYYYMMDD.
// Return respective components if pointers are passed in.
int date_julian_to_ymd(int j, int* y, int* m, int* d);

// Get the date of Easter for a given year.
// Return a value of the form YYYYMMDD.
// Return respective components if pointers are passed in.
int date_easter(int y, int* m, int* d);

#endif
