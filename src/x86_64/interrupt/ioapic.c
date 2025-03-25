/*
IOAPIC (I/O Advanced Programmable Interrupt Controller) is a part of the Intel APIC architecture. 
It is used to route interrupts from I/O devices to the CPU. 
It is a separate chip on the motherboard and is connected to the CPU through the system bus

https://wiki.osdev.org/IOAPIC
*/

#include "../../lib/stdio.h"
#include "../../driver/io/ports.h"
#include "ioapic.h"

// Offsets for IOAPIC registers
#define IOAPIC_REGSEL   0x00
#define IOAPIC_WINDOW   0x10

// The above registers used to detect below registers
#define IOAPICID        0x00    // E/W
#define IOAPICVER       0x01    // R/O
#define IOAPICARB       0x02    // R/O
#define IOAPICREDTBL    0x10    // R/W

// IOAPIC Redirection Entry Flags
#define IOAPIC_FIXED        (0 << 8)    // Fixed delivery mode
#define IOAPIC_LOW_ACTIVE   (1 << 13)   // Active low polarity
#define IOAPIC_LEVEL_TRIG   (1 << 15)   // Level-triggered mode


// Global IOAPIC base address (set in madt.c)
uint32_t ioapic_addr;   

// Write to an IOAPIC register (indirect access)
void ioapic_write_reg(uint32_t reg, uint32_t value) {
    // Write the register index to the register select
    *((volatile uint32_t*)(ioapic_addr + IOAPIC_REGSEL)) = reg;
    // Write the value to the window register
    *((volatile uint32_t*)(ioapic_addr + IOAPIC_WINDOW)) = value;
}

// Read from an IOAPIC register (indirect access)
uint32_t ioapic_read_reg(uint32_t reg) {
    // Write the register index to the register select
    *((volatile uint32_t*)(ioapic_addr + IOAPIC_REGSEL)) = reg;
    // Read the value from the window register
    return *((volatile uint32_t*)(ioapic_addr + IOAPIC_WINDOW));
}

void enable_ioapic_mode() {
    // These outb() calls appear to enable IOAPIC mode;
    // ensure that the I/O port addresses and values are correct for KeblaOS.
    outb(0x22, 0x70);  // Select the IOAPIC indirect register
    outb(0x23, 0x01);  // Set the IOAPIC indirect register value to 1
}


void ioapic_route_irq(uint8_t irq, uint8_t apic_id, uint8_t vector, uint32_t flags) {
    uint32_t reg_low  = IOAPICREDTBL + irq * 2;
    uint32_t reg_high = reg_low + 1;

    // High DWORD: Set destination APIC ID
    ioapic_write_reg(reg_high, (apic_id << 24));

    // Low DWORD: Vector + Flags (e.g., trigger mode, polarity)
    uint32_t low = vector | flags;
    ioapic_write_reg(reg_low, low);
}


