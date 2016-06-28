#pragma once

static const int MAX_TIME_TEXT_LEN = 9;

time_t local_mktime(struct tm *timeptr);

char *format_seconds(long seconds, char *buffer);