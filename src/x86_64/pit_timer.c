/*
https://wiki.osdev.org/Programmable_Interval_Timer
*/

#include "pit_timer.h"

uint64_t max_value = 18446744073709551615ULL; // Unsigned long long literal

const uint32_t freq = 1193180; // ticks per second
// PIT Frequency = 119318.16666 Mhz
// uint64_t divisor = 1193182 / freq; // 1.1931816666 ~ MHz 1.193182 MHz
uint64_t divisor = 1000;

uint64_t ticks = 0;

uint32_t sec = 0;

void timerHadler(registers_t *regs){
    if(ticks >= max_value){
        ticks = 0;
    }
    ticks++;

    if (ticks % freq == 0) {
        sec++;

        // Calculate hours, minutes, and seconds
        uint32_t hours = sec / 3600;
        uint32_t minutes = (sec % 3600) / 60;
        uint32_t seconds = sec % 60;

       // Display time in HH:MM:SS format at position (0, 0)
       // print_dec(sec);
    }
}


void init_timer(){
    ticks = 0;
    interrupt_install_handler(0, &timerHadler);

    // 0011 0110
    outb(PIT_COMMAND_PORT, 0x36);
    outb(PIT_CHANNEL_0, (uint8_t)(divisor & 0xFF));  // FF = 1111 1111 => last eight digit
    outb(PIT_CHANNEL_0, (uint8_t)((divisor >> 8) & 0xFF)); 
    print("Successfully PIT timer initialized.\n");
}


void delay(uint32_t ms) {
    uint64_t endTicks = ticks + (ms * freq / 1000);  // Convert milliseconds to ticks
    while (ticks < endTicks) {
        // Wait until the desired number of ticks has passed
        print_dec(ticks);
        print("\n");
    }
}



