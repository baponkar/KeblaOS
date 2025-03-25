/*
HPET(High Precision Event Timer)
https://wiki.osdev.org/HPET
*/

#include "../../process/types.h"
#include "../../process/thread.h"
#include "../../process/process.h"
#include "../../acpi/descriptor_table/hpet.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../util/util.h"
#include "../interrupt/interrupt.h"
#include "../interrupt/apic.h"
#include "../interrupt/ioapic.h"

#include "hpet_timer.h"

#define HPET_VECTOR 49

// HPET Register Offsets
#define HPET_CAPABILITIES     0x000   // Capabilities and ID Register
#define HPET_CONFIG           0x010   // General Configuration Register
#define HPET_INTERRUPT_STATUS 0x020   // Interrupt Status Register
#define HPET_TIMER0_CONFIG    0x100   // Timer 0 Configuration and Capability Register
#define HPET_TIMER0_COMPARATOR 0x108  // Timer 0 Comparator Register
#define HPET_TIMER0_FSB_ROUTE 0x110   // Timer 0 FSB Interrupt Route Register

#define MAX_HPET_TICKS  0xFFFFFFFFFFFFFFFF

extern bool has_apic();

extern hpet_t *hpet_addr; // Defined in apci.c
extern void restore_cpu_state(registers_t* registers);
extern process_t *current_process; 
volatile uint64_t hpet_ticks = 0; 

void init_hpet() {
    uint32_t *hpet = (uint32_t *) hpet_addr;
    if (!hpet_addr) {
        printf("HPET address not initialized!\n");
        return;
    }

    // Enable HPET and set up Timer 0
    uint64_t cap_reg = ioapic_read((uint32_t)hpet + HPET_CAPABILITIES) | 
                   ((uint64_t)ioapic_read((uint32_t)hpet + HPET_CAPABILITIES + 4) << 32);
    uint32_t period = (uint32_t)(cap_reg && 0xFFFFFFFF); // Extract lower 32 bits
    if (period == 0) {
        printf("HPET period is invalid!\n");
        return;
    }

    // Calculate ticks per millisecond
    uint64_t freq = 1000000000000000ULL / period; // Convert to Hz
    uint64_t ticks_per_ms = freq / 1000;

    // Enable HPET and Timer 0
    ioapic_write((uint32_t)hpet + HPET_CONFIG, 0x1); // Global enable
    ioapic_write((uint32_t)hpet + HPET_TIMER0_CONFIG, 
              (1 << 2) | (1 << 3) | (HPET_VECTOR)); // Periodic mode, enable interrupt, vector 48
    ioapic_write((uint32_t)hpet + HPET_TIMER0_COMPARATOR, ticks_per_ms);
}


void hpet_irq_handler(registers_t *regs) {
    if(hpet_ticks >= MAX_HPET_TICKS) hpet_ticks = 0;
    hpet_ticks++;
    
    printf("HPET Ticks: %d\n", hpet_ticks);

    apic_send_eoi(); // Acknowledge the interrupt
}


void hpet_init() {
    if (!has_apic()) {
        printf("APIC not supported!\n");
        return;
    }
    
    init_hpet();           // Configure HPET
    enable_ioapic_mode();
    interrupt_install_handler(HPET_VECTOR - 32, hpet_irq_handler); // Install handler for vector 48
    printf("HPET Timer Started\n");
}

void hpet_sleep(uint32_t ms) {
    if (!hpet_addr) {
        printf("HPET address not initialized!\n");
        return;
    }

    uint32_t *hpet = (uint32_t *) hpet_addr;

    // Retrieve the period and calculate frequency
    uint64_t cap_reg = ioapic_read((uint32_t)hpet + HPET_CAPABILITIES) | ((uint64_t)ioapic_read((uint32_t)hpet + HPET_CAPABILITIES + 4) << 32);
    uint32_t period = (uint32_t)(cap_reg >> 32);

    if (period == 0) {
        printf("HPET period is invalid!\n");
        return;
    }

    uint64_t freq = 1000000000000000ULL / period; // Convert to Hz
    uint64_t ticks_per_ms = freq / 1000;

    // Current HPET main counter value
    uint64_t start_ticks = ioapic_read((uint32_t)hpet + 0xF0) | 
                           ((uint64_t)ioapic_read((uint32_t)hpet + 0xF4) << 32);

    uint64_t target_ticks = start_ticks + (ms * ticks_per_ms);

    // Poll until the target tick count is reached
    while (1) {
        uint64_t current_ticks = ioapic_read((uint32_t)hpet + 0xF0) | 
                                 ((uint64_t)ioapic_read((uint32_t)hpet + 0xF4) << 32);
        if (current_ticks >= target_ticks) {
            break;
        }
    }
}

