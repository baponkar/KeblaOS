/*
    https://wiki.osdev.org/Programmable_Interval_Timer
*/

#include "../../process/process.h"
#include "../interrupt/pic.h"
#include "../interrupt/apic.h"
#include "tsc.h"
#include "../../lib/stdio.h"
#include "../../driver/io/ports.h"
#include "../interrupt/interrupt.h"
#include "../../util/util.h"

#include "pit_timer.h"


#define CHANEL0_DATA_PORT 0x40
#define CHANEL1_DATA_PORT 0x41
#define CHANEL2_DATA_PORT 0x42
#define COMMAND_PORT      0x43
#define PIT_FREQUENCY     1193180   // 1.193182 MHz = 1193182 Hz

volatile uint64_t ticks = 0;
uint32_t frequency = 100;           // 100 Hz -> 10 ms per tick



void pit_timerHandler(registers_t *regs) {
    ticks++;

    if (ticks % 100 == 0) printf("PIT Tick no : %d\n", ticks);  // Prints every 1 sec

    outb(0x20, 0x20); // Send End of Interrupt (EOI) to the PIC
}


void init_pit_timer() {

    interrupt_install_handler(0, &pit_timerHandler);            // IRQ0 for PIT timer

    uint16_t divisor = (uint16_t) PIT_FREQUENCY / frequency;    // Divisor max value 65535

    // Command port: 0x36 for repeating square wave mode
    outb(COMMAND_PORT, 0x36);  
    // Send the frequency divisor (LSB first, then MSB)
    outb(CHANEL0_DATA_PORT, (uint8_t) (divisor & 0xFF) );         // Low byte of divisor
    outb(CHANEL0_DATA_PORT, (uint8_t) ((divisor & 0xFF00) >> 8));  // High byte of divisor
    
    printf("Successfully PIT timer initialized.\n");
}


// Delay function using busy-wait based on ticks
void delay(uint32_t millisec) {
    uint64_t endTicks = ticks + ((frequency * millisec) / 1000);  // Convert milliseconds to ticks

    while (ticks < endTicks){             // Busy wait for the timer to reach the desired tick count
        asm volatile ("hlt");
    } 
}

