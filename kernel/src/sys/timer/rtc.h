
#include <stdint.h>

typedef struct rtc_time{ // 6 Byte
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t days;
    uint8_t months;
    uint8_t years;
} rtc_time_t;


uint8_t read_rtc_register(uint8_t reg);
uint8_t bcd_to_bin(uint8_t value);
void print_current_time();

void rtc_init();

rtc_time_t *get_rtc_time();


