#include <pebble.h>
#include "util.h"


static int isleap(unsigned yr)
{
  return yr % 400 == 0 || (yr % 4 == 0 && yr % 100 != 0);
}

static unsigned months_to_days(unsigned month)
{
  return (month * 3057 - 3007) / 100;
}

static long years_to_days (unsigned yr)
{
  return yr * 365L + yr / 4 - yr / 100 + yr / 400;
}

static long ymd_to_scalar(unsigned yr, unsigned mo, unsigned day) {
  long scalar;

  scalar = day + months_to_days(mo);
  if ( mo > 2 )                         /* adjust if past February */
     scalar -= isleap(yr) ? 1 : 2;
  yr--;
  scalar += years_to_days(yr);
  return (scalar);
}

time_t local_mktime(struct tm *timeptr) {
  time_t tt;
  
  if ((timeptr->tm_year < 70) || (timeptr->tm_year > 120)) {
    tt = (time_t)-1;
  } else {
    tt = ymd_to_scalar(timeptr->tm_year + 1900,
                       timeptr->tm_mon + 1,
                       timeptr->tm_mday) - ymd_to_scalar(1970, 1, 1);
        tt = tt * 24 + timeptr->tm_hour;
       tt = tt * 60 + timeptr->tm_min;
        tt = tt * 60 + timeptr->tm_sec;
    }
  *timeptr = *gmtime(&tt);
  return (tt);
}

char *format_seconds(long seconds, char *buffer) {
  bool negative = seconds < 0;
  if (negative) {
    seconds *= -1;
    buffer[0] = '-';
  }
  
  int hours = seconds / 3600;
  int minutes = (seconds % 3600) / 60;
  struct tm t = { .tm_sec = seconds % 60, .tm_min = minutes, .tm_hour = hours };
  char *offset = negative ? buffer + sizeof(char) : buffer;
  if (hours > 0)
    strftime(offset, MAX_TIME_TEXT_LEN, "%H:%M:%S", &t);
  else
    strftime(offset, MAX_TIME_TEXT_LEN, "%M:%S", &t);
  
  return buffer;
}