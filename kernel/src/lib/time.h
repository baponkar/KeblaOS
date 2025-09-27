#ifndef _TIME_H
#define _TIME_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Clock IDs (POSIX-compatible values) */
#define CLOCK_REALTIME   0
#define CLOCK_MONOTONIC  1

/* struct timespec: seconds + nanoseconds */
struct timespec {
    int64_t tv_sec;   /* seconds */
    int64_t tv_nsec;  /* nanoseconds */
};

/* struct timeval: seconds + microseconds */
struct timeval {
    int64_t tv_sec;   /* seconds */
    int64_t tv_usec;  /* microseconds */
};

/* struct timezone: mostly obsolete, but still in POSIX */
struct timezone {
    int tz_minuteswest; /* minutes west of Greenwich */
    int tz_dsttime;     /* type of DST correction */
};

/* struct tms: used by times() syscall */
struct tms {
    uint64_t tms_utime;   /* user CPU time */
    uint64_t tms_stime;   /* system CPU time */
    uint64_t tms_cutime;  /* user CPU time of children */
    uint64_t tms_cstime;  /* system CPU time of children */
};

typedef long time_t;
typedef long clock_t;

struct tm {
    int tm_sec;   // seconds [0,59]
    int tm_min;   // minutes [0,59]
    int tm_hour;  // hours [0,23]
    int tm_mday;  // day of month [1,31]
    int tm_mon;   // month [0,11]
    int tm_year;  // years since 1900
    int tm_wday;  // day of week [0,6] (Sunday = 0)
    int tm_yday;  // day of year [0,365]
    int tm_isdst; // daylight savings flag
};


time_t get_time();
uint64_t get_uptime_seconds(uint8_t cpu_id);

// Time functions
time_t time(time_t *t);
char *ctime(const time_t *t);           // Convert time to string
struct tm *gmtime(const time_t *t);     // Convert time to UTC tm struct
struct tm *localtime(const time_t *t);  // Convert time to local tm struct
size_t strftime(char *s, size_t max, const char *format, const struct tm *tm);  // 

#define CLOCKS_PER_SEC 1000000
clock_t clock(void);
#endif


void sleep_seconds(uint8_t cpu_id, uint64_t seconds);
void usleep(uint8_t cpu_id, uint64_t usec);

void test_time_functions();








