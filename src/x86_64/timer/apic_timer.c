/*
APIC TIMER:

https://wiki.osdev.org/APIC_Timer
https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/08_Timers.md

*/

#include "../../process/types.h"
#include "../../process/thread.h"
#include "../../process/process.h"
#include "../interrupt/pic.h"
#include "../interrupt/apic.h"
#include "../interrupt/interrupt.h"
#include "../../lib/string.h"
#include "../../util/util.h"
#include "../../lib/stdio.h"

#include "tsc.h"
#include "pit_timer.h"

#include "apic_timer.h"


#define MAX_APIC_TICKS 0xFFFFFFFFFFFFFFFF

#define LAPIC_BASE 0xFEE00000
#define APIC_TIMER_VECTOR  0x30  // 48: Interrupt vector for timer

#define LAPIC_TPR                    (LAPIC_BASE + 0x80)   // Task Priority Register Offset
#define APIC_REGISTER_TIMER_DIV      (LAPIC_BASE + 0x3E0)  // APIC Timer Divide Configuration Register
#define APIC_REGISTER_TIMER_INITCNT  (LAPIC_BASE + 0x380)  // APIC Timer Initial Count Register
#define APIC_REGISTER_TIMER_CURRCNT  (LAPIC_BASE + 0x390)  // APIC Timer Current Count Register
#define APIC_REGISTER_LVT_TIMER      (LAPIC_BASE + 0x320)  // APIC Local Vector Table (LVT) Timer Register

#define APIC_LVT_TIMER_MODE_PERIODIC (1 << 17)             // Periodic mode bit
#define APIC_LVT_INT_MASKED          (1 << 16)             // Mask interrupt


extern uint64_t pit_ticks;
uint64_t cpu_frequency_hz = 1600000000;                    // Cached CPU frequency in Hz
volatile uint64_t apic_ticks = 0;
volatile uint64_t apic_timer_ticks_per_ms = 0;



void apic_start_timer(uint32_t initial_count) {

    // Ensure the Local APIC is enabled
    mmio_write(LAPIC_TPR, 0x0); // Accept all interrupts

    // Set APIC timer to use divider 16
    mmio_write(APIC_REGISTER_TIMER_DIV, 0x3);
    
    // Set APIC initial count to max
    mmio_write(APIC_REGISTER_TIMER_INITCNT, initial_count);

    // Set APIC Timer Mode (Periodic Mode = 0x20000, One-shot = 0x0)
    mmio_write(APIC_REGISTER_LVT_TIMER, APIC_TIMER_VECTOR | 0x00000);  // Vector 48, one-shot mode
    
}

void calibrate_apic_timer() {
    uint64_t start_ticks = pit_ticks;  // Capture PIT ticks

    // Wait for 10 PIT ticks (100 ms delay)
    // while (pit_ticks < start_ticks + 10){
    //     printf("pit_ticks: %d\n", pit_ticks);
    // }

    pit_sleep(100); // Sleep PIT timer for 100 millisecond which can hold 10 PIT interrupt

    // Read remaining APIC timer count
    uint32_t end_count = mmio_read(APIC_REGISTER_TIMER_CURRCNT);

    // Calculate APIC Timer ticks per millisecond
    apic_timer_ticks_per_ms = (0xFFFFFFFF - end_count) / 100;

    printf("APIC Timer Frequency: %d ticks/ms\n", apic_timer_ticks_per_ms);
}



void apic_timer_handler(registers_t *regs) {

    if(apic_ticks >= MAX_APIC_TICKS) apic_ticks = 0;
    apic_ticks++;
    apic_send_eoi();

    printf("APIC Tick: %d\n", apic_ticks);

}

void init_apic_timer(uint32_t interval_ms) {// Start APIC timer with a large count
    apic_start_timer(0xFFFFFFFF);
    calibrate_apic_timer();

    uint32_t apic_count = apic_timer_ticks_per_ms * interval_ms;
    
    // Set APIC Timer for periodic interrupts
    mmio_write(APIC_REGISTER_LVT_TIMER, APIC_TIMER_VECTOR | 0x20000);   // Vector 32, Periodic Mode
    mmio_write(APIC_REGISTER_TIMER_DIV , 0x3);           // Set divisor
    mmio_write(APIC_REGISTER_TIMER_INITCNT, apic_count);

    interrupt_install_handler((APIC_TIMER_VECTOR - 32), &apic_timer_handler);

    printf("APIC Timer initialized with %d ms interval.\n", interval_ms);
}


void apic_delay(uint32_t milliseconds) {
    uint32_t target_ticks = apic_ticks + (milliseconds / 10);  // Convert ms to 10ms ticks
    while (apic_ticks < target_ticks);
}


