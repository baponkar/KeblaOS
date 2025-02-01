

#include "../../lib/stdio.h"
#include "../../driver/ports.h"

#include "rtc.h"
// Ports for RTC
#define RTC_COMMAND_PORT 0x70
#define RTC_DATA_PORT 0x71

// RTC Registers
#define RTC_SECONDS     0x00
#define RTC_MINUTES     0x02
#define RTC_HOURS       0x04
#define RTC_DAY         0x07
#define RTC_MONTH       0x08
#define RTC_YEAR        0x09


// Helper function to read a byte from the RTC
uint8_t read_rtc_register(uint8_t reg) {
    outb(RTC_COMMAND_PORT, reg);  // Write register number to RTC command port
    return inb(RTC_DATA_PORT);    // Read the value from the data port
}


// Convert BCD to binary (if needed)
uint8_t bcd_to_bin(uint8_t value) {
    return ((value >> 4) * 10) + (value & 0x0F);
}


// Function to read and print the current time
void print_current_time() {
    uint8_t seconds = read_rtc_register(RTC_SECONDS);
    uint8_t minutes = read_rtc_register(RTC_MINUTES);
    uint8_t hours   = read_rtc_register(RTC_HOURS);
    uint8_t day     = read_rtc_register(RTC_DAY);
    uint8_t month   = read_rtc_register(RTC_MONTH);
    uint8_t year    = read_rtc_register(RTC_YEAR);

    // Convert BCD to binary (RTC typically uses BCD format)
    seconds = bcd_to_bin(seconds);
    minutes = bcd_to_bin(minutes);
    hours   = bcd_to_bin(hours);
    day     = bcd_to_bin(day);
    month   = bcd_to_bin(month);
    year    = bcd_to_bin(year);

    // RTC only provides the last two digits of the year, assume 20xx
    year += 2000;

    // Print the time in HH:MM:SS DD/MM/YYYY format
    printf("Current Time: %d:%d:%d %d/%d/%d\n", hours, minutes, seconds, day, month, year);
}


