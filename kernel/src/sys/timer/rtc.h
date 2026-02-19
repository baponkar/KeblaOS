
#include <stdint.h>

typedef struct rtc_time{ 
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t days;
    uint8_t months;
    uint8_t years;
} rtc_time_t;   // 6 bytes




void rtc_init();
rtc_time_t *get_rtc_time();


void print_current_rtc_time();




