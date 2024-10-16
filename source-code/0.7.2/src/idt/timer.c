
#include "timer.h"

uint32_t ticks;
const uint32_t freq = 1000; // in milli seconds

void onIrq0(registers_t *regs){
    ticks++;
    // Update the time on screen every 1000 ticks (i.e., 1 second)
    if (ticks % 1000 == 0) {
        // update_time_on_screen();    // This function is in rtc.h
        // pc_speaker_enable_sound(SA_UP_FREQ);
    }
}

void initTimer(){
    ticks = 0;
    irq_install_handler(0, &onIrq0);

    //119318.16666 Mhz
    uint32_t divisor = 1193180 / freq; // 1.1931816666 MHz

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
    }
}