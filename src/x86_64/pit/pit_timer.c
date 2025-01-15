/*
https://wiki.osdev.org/Programmable_Interval_Timer
*/

#include "pit_timer.h"


volatile uint64_t ticks = 0;
const uint64_t max_value = 18446744073709551615ULL; // Unsigned long long literal
const uint32_t freq = 1193180;  // PIT frequency (Hz)
uint16_t divisor = 1000;        // Default divisor for ~1ms tick rate

void timerHandler(registers_t *regs) {
    (void)regs; // Suppress unused parameter warning if not used.
    ticks++;

    if (ticks >= max_value) {
        ticks = 0;
    }
    
    // print("tick ");
}

void init_timer() {
    ticks = 0;
    interrupt_install_handler(0, &timerHandler);  // IRQ0 for PIT timer

    // Set PIT frequency using divisor (frequency = 1193180 / divisor)
    divisor = freq / 1000;  // Set divisor for ~1ms tick rate
    outb(0x43, 0x36);  // Command port: 0x36 for repeating square wave mode
    outb(0x40, (uint8_t)(divisor & 0xFF));      // Low byte of divisor
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF)); // High byte of divisor

    print("PIT timer successfully initialized.\n");
}

// Delay function using busy-wait based on ticks
void delay(uint32_t ms) {
    uint64_t endTicks = ticks + (ms * (freq / divisor) / 1000);  // Convert milliseconds to ticks
    while (ticks < endTicks) {
        // Busy wait for the timer to reach the desired tick count
    }
    // print("delay function completed\n");
}


