
#include "../sys/timer/rtc.h"
#include "../sys/timer/apic_timer.h"

#include "../lib/stdio.h"
#include "../lib/string.h"

#include "time.h"



// Days per month
static int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };


// Leap year checker
static int is_leap(int y) {
    return (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0));
}

static time_t rtc_to_time_t(rtc_time_t *rt) {
    
    if(!rt){
        return 0;
    }

    time_t t = 0;

    // Years since 1970
    for (int y = 1970; y < rt->years + 2000; y++) {
        t += 365 * 24 * 3600;           // Total seconds in a year
        if(is_leap(y)) t += 24 * 3600;  // add more one day
    }

    // Months this year
    for (int m = 1; m < rt->months; m++) {
        t += days_in_month[m - 1] * 24 * 3600;
        if (m == 2 && is_leap(rt->years)) t += 24 * 3600;
    }

    // Days, hours, minutes, seconds
    t += (rt->days - 1) * 24 * 3600;
    t += rt->hours * 3600;
    t += rt->minutes * 60;
    t += rt->seconds;

    return t;
}

time_t get_time(){
    return rtc_to_time_t(get_rtc_time());   // return total seconds from  1970-01-01 00:00:00 UTC
}


//  Returns the current calendar time as a time_t (seconds since Epoch: 1970-01-01 00:00:00 UTC)
time_t time(time_t *t) {
    time_t current = get_time();
    if (t) {
        *t = current;
    }
    return current;
}


// Converts time_t to a struct tm in UTC (Coordinated Universal Time).
struct tm *gmtime(const time_t *t) {
    static struct tm tm;
    time_t seconds = *t;

    tm.tm_sec = seconds % 60;
    seconds /= 60;
    tm.tm_min = seconds % 60;
    seconds /= 60;
    tm.tm_hour = seconds % 24;
    seconds /= 24;

    // Count days since 1970-01-01
    int year = 1970;
    while (1) {
        int days_in_year = is_leap(year) ? 366 : 365;
        if (seconds >= days_in_year) {
            seconds -= days_in_year;
            year++;
        } else {
            break;
        }
    }

    tm.tm_year = year - 1900;
    tm.tm_yday = seconds;

    int month = 0;
    while (1) {
        int days = days_in_month[month];
        if (month == 1 && is_leap(year)) days += 1;
        if (seconds >= days) {
            seconds -= days;
            month++;
        } else {
            break;
        }
    }

    tm.tm_mon = month;
    tm.tm_mday = seconds + 1;
    tm.tm_wday = (4 + *t / 86400) % 7; // 1970-01-01 was a Thursday
    tm.tm_isdst = 0;

    return &tm;
}


// Converts time_t to struct tm in local time zone
#define LOCAL_TZ_OFFSET 19800   // UTC+5:30 in seconds

struct tm *localtime(const time_t *t) {
    static time_t adjusted;
    adjusted = *t + LOCAL_TZ_OFFSET;
    return gmtime(&adjusted);
}



// Converts a time_t value to a human-readable string
char *ctime(const time_t *t) {
    static char buf[26];
    struct tm *tm = localtime(t);
    static const char *wday_name[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    static const char *mon_name[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

    snprintf(buf, sizeof(buf), "%s %s %d %d:%d:%d %d\n",
        wday_name[tm->tm_wday], mon_name[tm->tm_mon], tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec, 1900 + tm->tm_year);

    return buf;
}



#define MAX_CPUS 256
extern volatile uint64_t apic_ticks[MAX_CPUS];
extern volatile uint64_t apic_timer_ticks_per_ms;


uint64_t get_uptime_seconds(uint8_t cpu_id) {
    // return apic_ticks[cpu_id] / (apic_timer_ticks_per_ms * 1000);
    return (uint64_t) get_time();   // using RTC
}

// Returns processor time consumed by the program since it started.
clock_t clock(void) {
    return get_uptime_seconds(0) * CLOCKS_PER_SEC;
}


// Formats a struct tm into a string according to a specified format (like printf for time).
size_t strftime(char *s, size_t max, const char *format, const struct tm *tm) {
    static const char *wday_name[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    static const char *mon_name[]  = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

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


void sleep_seconds(uint8_t cpu_id, uint64_t seconds) {
    uint64_t start = get_uptime_seconds(cpu_id);
    while ((get_uptime_seconds(cpu_id) - start) < seconds) {
        // asm volatile("hlt"); // save CPU cycles
    }
}


void usleep(uint8_t cpu_id, uint64_t usec) {
    uint64_t start_ms = get_uptime_seconds(cpu_id) * 1000; // convert seconds to ms
    uint64_t target_ms = start_ms + usec / 1000;     // target time in ms

    while ((get_uptime_seconds(cpu_id) * 1000) < target_ms) {
        // asm volatile("hlt"); // reduce CPU usage
    }
}



void test_time_functions() {

    // Get current time
    time_t now = time(NULL);
    printf("Epoch Time (time_t): %d\n", now);

    // ctime
    char *ctime_str = ctime(&now);
    printf("ctime: %s", ctime_str);  // already includes newline

    // greenich mean time
    struct tm *gmt = gmtime(&now);
    printf("gmtime: %d-%d-%d %d:%d:%d (UTC)\n",
           1900 + gmt->tm_year, gmt->tm_mon + 1, gmt->tm_mday,
           gmt->tm_hour, gmt->tm_min, gmt->tm_sec);

    // localtime
    struct tm *loc = localtime(&now);
    printf("localtime: %d-%d-%d %d:%d:%d (Local)\n",
           1900 + loc->tm_year, loc->tm_mon + 1, loc->tm_mday,
           loc->tm_hour, loc->tm_min, loc->tm_sec);

    char buf[128];

    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S UTC", loc);
    printf("%s\n", buf);

}






