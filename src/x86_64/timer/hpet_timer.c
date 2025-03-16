/*
HPET(High Precision Event Timer)
https://wiki.osdev.org/HPET
*/

#include "../../process/types.h"
#include "../../process/thread.h"
#include "../../process/process.h"

#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../util/util.h"
#include "../interrupt/interrupt.h"
#include "../interrupt/apic.h"

#include "hpet_timer.h"

#define HPET_VECTOR 48

// HPET Register Offsets
#define HPET_CAPABILITIES     0x000   // Capabilities and ID Register
#define HPET_CONFIG           0x010   // General Configuration Register
#define HPET_INTERRUPT_STATUS 0x020   // Interrupt Status Register
#define HPET_TIMER0_CONFIG    0x100   // Timer 0 Configuration and Capability Register
#define HPET_TIMER0_COMPARATOR 0x108  // Timer 0 Comparator Register
#define HPET_TIMER0_FSB_ROUTE 0x110   // Timer 0 FSB Interrupt Route Register

// Map HPET physical address to virtual memory
extern void *hpet_addr;
volatile uint32_t *hpet;

extern void restore_cpu_state(registers_t* registers);
extern process_t *current_process; 

#define MAX_HPET_TICKS  0xFFFFFFFFFFFFFFFF

volatile uint64_t hpet_ticks = 0; 

void init_hpet() {
    hpet = (uint32_t *) hpet_addr;
    if (!hpet_addr) {
        printf("HPET address not initialized!\n");
        return;
    }

    // Enable HPET and set up Timer 0
    uint64_t cap_reg = mmio_read((uint32_t)hpet + HPET_CAPABILITIES) | 
                   ((uint64_t)mmio_read((uint32_t)hpet + HPET_CAPABILITIES + 4) << 32);
    uint32_t period = (uint32_t)(cap_reg & 0xFFFFFFFF); // Extract lower 32 bits
    if (period == 0) {
        printf("HPET period is invalid!\n");
        return;
    }

    // Calculate ticks per millisecond
    uint64_t freq = 1000000000000000ULL / period; // Convert to Hz
    uint64_t ticks_per_ms = freq / 1000;

    // Enable HPET and Timer 0
    mmio_write((uint32_t)hpet + HPET_CONFIG, 0x1); // Global enable
    mmio_write((uint32_t)hpet + HPET_TIMER0_CONFIG, 
              (1 << 2) | (1 << 3) | (0x30)); // Periodic mode, enable interrupt, vector 48
    mmio_write((uint32_t)hpet + HPET_TIMER0_COMPARATOR, ticks_per_ms);
}


void hpet_irq_handler(registers_t *regs) {
    if(hpet_ticks >= MAX_HPET_TICKS) hpet_ticks = 0;
    hpet_ticks++;
    apic_send_eoi(); // Acknowledge the interrupt

    printf("HPET Ticks: %d\n", hpet_ticks);

    // registers_t *new_regs = schedule(regs);
    // if(new_regs){
    //     // printf("=>current thread: %s, rip: %x, rsp: %x\n", 
    //     //     current_process->current_thread->name,  
    //     //     current_process->current_thread->registers.iret_rip,
    //     //     current_process->current_thread->registers.iret_rsp);
    //     restore_cpu_state(new_regs);
    // }
}

void route_hpet_interrupt() {
    // Redirect HPET Timer 0 to APIC vector 48 (IRQ 2)
    enable_ioapic_mode();
    lapic_send_ipi(0, 0x30); // Optional: Test IPI to bootstrap CPU
    interrupt_install_handler(HPET_VECTOR - 32, hpet_irq_handler); // Install handler for vector 48
}


void start_hpet() {
    if (!has_apic()) {
        printf("APIC not supported!\n");
        return;
    }

    init_apic_interrupt(); // Enable APIC
    init_hpet();           // Configure HPET
    route_hpet_interrupt(); // Route interrupts via IOAPIC
    printf("HPET Timer Started\n");
}


