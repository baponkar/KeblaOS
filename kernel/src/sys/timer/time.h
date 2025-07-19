#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct tm {
  int tm_sec;   // seconds [0,60]
  int tm_min;   // minutes [0,59]
  int tm_hour;  // hour [0,23]
  int tm_mday;  // day of month [1,31]
  int tm_mon;   // months since January [0,11]
  int tm_year;  // years since 1900
  int tm_wday;  // days since Sunday [0,6]
  int tm_yday;  // days since Jan 1 [0,365]
  int tm_isdst; // daylight saving time flag
}tm_t;


typedef long time_t;     // or sometimes: typedef int64_t time_t;

// UNIX Epoch = 1970-01-01 00:00:00 UTC
time_t get_time();
uint64_t get_uptime_seconds(uint8_t cpu_id);




