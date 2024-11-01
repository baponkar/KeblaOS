#pragma once

#include "../driver/ports.h"
#include "../driver/vga.h"

#include "../idt/timer.h"

#include "../stdlib/stdint.h"
#include "../stdlib/stdio.h"


#define RTC_COMMAND_PORT 0x70       //Real-Time Clock (RTC) Command Ports
#define RTC_DATA_PORT 0x71          //Real-Time Clock (RTC) Data Ports

uint8_t read_rtc_register(int reg);
int bcd_to_binary(uint8_t bcd);
void get_rtc_time(uint8_t *hours, uint8_t *minutes, uint8_t *seconds);
void update_time_on_screen();



