#include <pebble.h>
#include "util.h"

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