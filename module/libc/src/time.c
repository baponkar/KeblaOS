
#include "../include/stdio.h"
#include "../include/string.h"

#include "../include/syscall.h"



/* ========== Basic Syscall Wrappers ========== */
time_t get_time() {
    return syscall_time(NULL);
}

uint64_t get_uptime_seconds(uint8_t cpu_id) {
    (void)cpu_id;  // if you don’t need per-CPU uptime, ignore cpu_id
    return syscall_get_uptime();
}

/* time(): return current epoch time */
time_t time(time_t *t) {
    time_t now = syscall_time(NULL);
    if (t) {
        *t = now;
    }
    return now;
}

/* clock(): CPU time used by this process */
clock_t clock(void) {
    struct tms buf;
    if (syscall_times(&buf) < 0) {
        return (clock_t)-1;
    }
    return buf.tms_utime + buf.tms_stime;  // simple version: user + system time
}

/* gettimeofday(): directly call syscall */
int gettimeofday(struct timeval *tv, struct timezone *tz) {
    return syscall_gettimeofday(tv, tz);
}

/* clock_gettime(): support CLOCK_REALTIME & CLOCK_MONOTONIC */
int clock_gettime(int clk_id, struct timespec *tp) {
    return syscall_clock_gettime(clk_id, tp);
}

/* ========== Conversions ========== */

/* Very basic ctime() implementation */
char *ctime(const time_t *t) {
    static char buf[26]; // e.g. "Wed Jun 30 21:49:08 1993\n\0"
    struct tm *tm = localtime(t);
    if (!tm) return NULL;

    // crude formatting: YYYY-MM-DD HH:MM:SS
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d\n",
             tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
             tm->tm_hour, tm->tm_min, tm->tm_sec);
    return buf;
}

/* gmtime(): crude conversion (UTC) */
struct tm *gmtime(const time_t *t) {
    static struct tm tm_buf;
    time_t raw = *t;

    // simple conversion: seconds -> tm
    // NOTE: This is *very naive* (no leap years, DST, etc.)
    time_t days = raw / 86400;
    tm_buf.tm_sec  = raw % 60;
    tm_buf.tm_min  = (raw / 60) % 60;
    tm_buf.tm_hour = (raw / 3600) % 24;
    tm_buf.tm_mday = (days % 30) + 1;
    tm_buf.tm_mon  = (days % 365) / 30;
    tm_buf.tm_year = 70 + days / 365;   // since 1970
    tm_buf.tm_wday = (days + 4) % 7;    // 1970-01-01 was Thursday
    tm_buf.tm_yday = days % 365;
    tm_buf.tm_isdst = 0;
    return &tm_buf;
}

/* localtime(): for now, same as gmtime */
struct tm *localtime(const time_t *t) {
    return gmtime(t);  // no timezone/DST support yet
}

// Formats a struct tm into a string according to a specified format (like printf for time).
size_t strftime(char *s, size_t max, const char *format, const struct tm *tm) {
    static const char *wday_name[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    static const char *mon_name[]  = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                       "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

    char *out = s;
    size_t remaining = max;

    while (*format && remaining > 1) {
        if (*format == '%') {
            format++;
            char buf[32];
            const char *replacement = buf;

            switch (*format) {
                case 'Y':
                    snprintf(buf, sizeof(buf), "%d", 1900 + tm->tm_year);
                    break;
                case 'm':
                    snprintf(buf, sizeof(buf), "%d", tm->tm_mon + 1);
                    break;
                case 'd':
                    snprintf(buf, sizeof(buf), "%d", tm->tm_mday);
                    break;
                case 'H':
                    snprintf(buf, sizeof(buf), "%d", tm->tm_hour);
                    break;
                case 'M':
                    snprintf(buf, sizeof(buf), "%d", tm->tm_min);
                    break;
                case 'S':
                    snprintf(buf, sizeof(buf), "%d", tm->tm_sec);
                    break;
                case 'a':
                    replacement = wday_name[tm->tm_wday % 7];
                    break;
                case 'b':
                    replacement = mon_name[tm->tm_mon % 12];
                    break;
                case 'F':
                    snprintf(buf, sizeof(buf), "%d-%d-%d", 1900 + tm->tm_year, tm->tm_mon + 1, tm->tm_mday);
                    break;
                case 'T':
                    snprintf(buf, sizeof(buf), "%d:%d:%d", tm->tm_hour, tm->tm_min, tm->tm_sec);
                    break;
                case '%':
                    buf[0] = '%';
                    buf[1] = '\0';
                    break;
                default:
                    // Unknown specifier — treat as literal
                    buf[0] = '%';
                    buf[1] = *format;
                    buf[2] = '\0';
                    break;
            }

            size_t len = strlen(replacement);
            if (len >= remaining) break;

            memcpy(out, replacement, len);
            out += len;
            remaining -= len;

            format++; // ✅ Advance past specifier
        } else {
            *out++ = *format++;
            remaining--;
        }
    }

    *out = '\0';
    return out - s;
}

void sleep_seconds(uint64_t seconds) {
    uint64_t start = syscall_get_uptime();
    while ((syscall_get_uptime() - start) < seconds) {
        // asm volatile("hlt"); // save CPU cycles
    }
}

void usleep(uint64_t usec) {
    uint64_t start_ms = syscall_get_uptime() * 1000; // convert seconds to ms
    uint64_t target_ms = start_ms + usec / 1000;     // target time in ms

    while ((syscall_get_uptime() * 1000) < target_ms) {
        // asm volatile("hlt"); // reduce CPU usage
    }
}



/* ========== Test Helper ========== */
void test_time_functions() {
    time_t now = time(NULL);
    printf("Epoch time: %d\n", now);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    printf("gettimeofday: %d sec, %d usec\n", (int)tv.tv_sec, (int)tv.tv_usec);

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    printf("clock_gettime: %d sec, %d nsec\n", (int)ts.tv_sec, (int)ts.tv_nsec);

    // char *str = ctime(&now);
    // if(!str){
    //     printf("ctime: %s\n", str);
    // }
}











