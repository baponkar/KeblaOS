/*
    https://wiki.osdev.org/Programmable_Interval_Timer
*/

#include "../../process/process.h"
#include "../interrupt/pic.h"
#include "../interrupt/apic.h"
#include "../../lib/stdio.h"
#include "../../driver/io/ports.h"
#include "../interrupt/interrupt.h"

#include "pic_timer.h"


volatile uint64_t ticks = 0;
const uint32_t freq = 1193180;  // PIT frequency (Hz)
uint16_t divisor = 1000;        // Default divisor for ~1ms tick rate


void pic_timerHandler(registers_t *regs) {
    // (void)regs; // Suppress unused parameter warning if not used.
    ticks++;

    // Uncomment this for debug purposes but be cautious with high-frequency output
    if (ticks % 10 == 0) {
        printf("Tick no : %d\n", ticks); // Assuming you have a function to printf 
        // schedule(regs);
    }

    outb(0x20, 0x20); // Send End of Interrupt (EOI) to the PIC
}


void init_pic_timer(uint16_t interval) {
    ticks = 0;
    interrupt_install_handler(0, &pic_timerHandler);  // IRQ0 for PIT timer

    // Set PIT frequency using divisor (frequency = 1193180 / divisor)
    divisor = (freq * interval) / 1000;  // Set divisor for ~1ms tick rate
    outb(0x43, 0x36);  // Command port: 0x36 for repeating square wave mode
    outb(0x40, (uint8_t)(divisor & 0xFF));      // Low byte of divisor
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF)); // High byte of divisor

    printf("Successfully PIC timer successfully initialized.\n");
}



// Delay function using busy-wait based on ticks
void delay(uint32_t ms) {
    uint64_t endTicks = ticks + (ms * (freq / divisor) / 1000);  // Convert milliseconds to ticks
    while (ticks < endTicks) {
        // Busy wait for the timer to reach the desired tick count
    }
    // print("delay function completed\n");
}

