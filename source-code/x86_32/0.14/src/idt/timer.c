#include "timer.h"


uint32_t ticks = 0;
const uint32_t freq = 1000; // ticks per second
uint32_t sec = 0;

void timerHadler(registers_t *regs){
    ticks++;
    if (ticks % freq == 0) {
        sec++;

        // Calculate hours, minutes, and seconds
        uint32_t hours = sec / 3600;
        uint32_t minutes = (sec % 3600) / 60;
        uint32_t seconds = sec % 60;

        // Display time in HH:MM:SS format at position (0, 0)
        // print_at("Time: ", 0, 0);
        // print_dec_at(hours, 6, 0);
        // print_at(":", 0, 8);
        // print_dec_at(minutes, 9, 0);
        // print_at(":", 11, 0);
        // print_dec_at(seconds, 12, 0);
        // print("\n");
    }
}


void initTimer(){
    ticks = 0;
    interrupt_install_handler(0, &timerHadler);

    //119318.16666 Mhz
    uint32_t divisor = 1193182 / freq; // 1.1931816666 ~ MHz 1.193182 MHz

    //0011 0110
    outb(PIT_COMMAND_PORT, 0x36);
    outb(PIT_CHANNEL_0, (uint8_t)(divisor & 0xFF));  // FF = 1111 1111 => last eight digit
    outb(PIT_CHANNEL_0, (uint8_t)((divisor >> 8) & 0xFF)); 
}



void delay(uint32_t ms) {
    uint64_t endTicks = ticks + (ms * freq / 1000);  // Convert milliseconds to ticks
    while (ticks < endTicks) {
        // Wait until the desired number of ticks has passed
        print_dec(ticks);
        print("\n");
    }
}