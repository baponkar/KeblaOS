#include "rtc.h"


uint8_t read_rtc_register(int reg) {
    outb(RTC_COMMAND_PORT, reg);  // Write the register number to port 0x70
    return inb(RTC_DATA_PORT);    // Read the value from port 0x71
}

// Convert BCD to binary
int bcd_to_binary(uint8_t bcd) {
    return ((bcd / 16) * 10) + (bcd & 0x0F);
}

// Function to check if RTC is updating
int rtc_is_updating() {
    outb(RTC_COMMAND_PORT, 0x0A);  // Select register 0x0A (status register A)
    return inb(RTC_DATA_PORT) & 0x80;  // Check bit 7 (Update In Progress - UIP)
}

void get_rtc_time(uint8_t *hours, uint8_t *minutes, uint8_t *seconds) {
    // Wait for RTC to stop updating
    while (rtc_is_updating());

    *seconds = bcd_to_binary(read_rtc_register(0x00)); // Seconds
    *minutes = bcd_to_binary(read_rtc_register(0x02)); // Minutes
    *hours   = bcd_to_binary(read_rtc_register(0x04)); // Hours
}



void update_time_on_screen() {
    uint8_t hours, minutes, seconds;
    
    // Get the current time
    get_rtc_time(&hours, &minutes, &seconds);

    // Display the time at a fixed position (e.g., row 0, starting from column 0)
    print_dec_at(hours, 0, 0);    // Display hours
    print_at(":",1,0);
    print_dec_at(minutes, 3, 0);  // Display minutes
    print_at(":",4,0);
    print_dec_at(seconds, 6, 0);  // Display seconds
}










