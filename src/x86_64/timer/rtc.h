
#include <stdint.h>



uint8_t read_rtc_register(uint8_t reg);
uint8_t bcd_to_bin(uint8_t value);
void print_current_time();

void rtc_init();
