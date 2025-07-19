
#include "rtc.h"
#include "apic_timer.h"
#include "time.h"


// Days per month
int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

// Leap year checker
static int is_leap(int y) {
    return (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0));
}


time_t rtc_to_time_t(rtc_time_t *rt) {
    
    if(!rt){
        return 0;
    }

    time_t t = 0;

    // Years since 1970
    for (int y = 1970; y < rt->years; y++) {
        t += 365 * 24 * 3600;
        if (is_leap(y)) t += 24 * 3600;
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

    return rtc_to_time_t(get_rtc_time());
}



#define MAX_CPUS 256
extern volatile uint64_t apic_ticks[MAX_CPUS];
extern volatile uint64_t apic_timer_ticks_per_ms;


uint64_t get_uptime_seconds(uint8_t cpu_id) {
    return apic_ticks[cpu_id] / (apic_timer_ticks_per_ms * 1000);
}


