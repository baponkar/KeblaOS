#include "../../pcb/process.h"
#include "../interrupt/pic.h"
#include "../interrupt/apic.h"
#include "../interrupt/interrupt.h"
#include "../../util/util.h"
#include "../../lib/stdio.h"

#include "apic_timer.h"

#define LAPIC_BASE 0xFEE00000
#define APIC_TIMER_VECTOR  0x20  // Interrupt vector for timer

#define APIC_REGISTER_TIMER_DIV      (LAPIC_BASE + 0x3E0)  // APIC Timer Divide Configuration Register
#define APIC_REGISTER_TIMER_INITCNT  (LAPIC_BASE + 0x380)  // APIC Timer Initial Count Register
#define APIC_REGISTER_TIMER_CURRCNT  (LAPIC_BASE + 0x390)  // APIC Timer Current Count Register
#define APIC_REGISTER_LVT_TIMER      (LAPIC_BASE + 0x320)  // APIC Local Vector Table (LVT) Timer Register

#define APIC_LVT_TIMER_MODE_PERIODIC (1 << 17)              // Periodic mode bit
#define APIC_LVT_INT_MASKED          (1 << 16)              // Mask interrupt


static inline uint64_t read_tsc() {
    uint32_t low, high;
    asm volatile ("rdtsc" : "=a"(low), "=d"(high)); // Read TSC
    return ((uint64_t)high << 32) | low;
}

void tsc_sleep(uint64_t microseconds) {
    uint64_t start = read_tsc();
    
    // Assuming a 3 GHz CPU, 1 cycle = 1 / 3 GHz = 0.333 ns
    // 1 ms = 3,000,000 cycles
    uint64_t cycles_to_wait = (microseconds * 3000); // Adjust based on CPU frequency

    while ((read_tsc() - start) < cycles_to_wait);
}


void apic_start_timer() {
    // Set APIC timer to use divider 16
    mmio_write(APIC_REGISTER_TIMER_DIV, 0x3);
    
    // Set APIC initial count to max
    mmio_write(APIC_REGISTER_TIMER_INITCNT, 0xFFFFFFFF);
    
    // Sleep for 10ms using TSC
    tsc_sleep(10000);
    
    // Stop APIC timer
    mmio_write(APIC_REGISTER_LVT_TIMER, APIC_LVT_INT_MASKED);
    
    // Calculate ticks in 10ms
    uint32_t ticksIn10ms = 0xFFFFFFFF - mmio_read(APIC_REGISTER_TIMER_CURRCNT);
    
    // Configure APIC timer in periodic mode with calculated ticks
    mmio_write(APIC_REGISTER_LVT_TIMER, APIC_TIMER_VECTOR | APIC_LVT_TIMER_MODE_PERIODIC);
    mmio_write(APIC_REGISTER_TIMER_DIV, 0x3);
    mmio_write(APIC_REGISTER_TIMER_INITCNT, ticksIn10ms);
}


void apic_delay(uint32_t milliseconds) {
    
    // Calculate ticks for the given delay based on 10ms calibration
    uint32_t ticks_per_ms = mmio_read(APIC_REGISTER_TIMER_CURRCNT) / 10;
    uint32_t ticks_to_wait = ticks_per_ms * milliseconds;

    // Configure APIC timer in one-shot mode
    mmio_write(APIC_REGISTER_LVT_TIMER, APIC_LVT_INT_MASKED);
    mmio_write(APIC_REGISTER_TIMER_INITCNT, ticks_to_wait);

    // Wait for timer to reach zero
    while (mmio_read(APIC_REGISTER_TIMER_CURRCNT) > 0);
}


int ticks1 = 0;

void apic_timer_handler(registers_t *regs) {
    ticks1++;
    // printf("APIC Timer Interrupt! : %d\n", ticks1); // Print message on each interrupt

    // printf("registers : %x\n", (uint64_t)regs);

    apic_send_eoi();
}

void init_apic_timer(){
    interrupt_install_handler(0, &apic_timer_handler);
    apic_start_timer();
    printf("APIC Timer enabled\n");
}






