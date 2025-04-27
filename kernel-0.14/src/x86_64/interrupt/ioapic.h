#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// IOAPIC Redirection Entry Flags
#define IOAPIC_FIXED        (0 << 8)    // Fixed delivery mode
#define IOAPIC_LOWEST       (1 << 8)    // Lowest priority delivery mode
#define IOAPIC_SMI          (2 << 8)    // System Management Interrupt
#define IOAPIC_NMI          (4 << 8)    // Non-Maskable Interrupt
#define IOAPIC_INIT         (5 << 8)    // INIT delivery mode
#define IOAPIC_EXTINT       (7 << 8)    // External Interrupt (for legacy PIC)

#define IOAPIC_LOW_ACTIVE   (1 << 13)   // Active low polarity
#define IOAPIC_HIGH_ACTIVE  (0 << 13)   // Active high polarity (default)

#define IOAPIC_EDGE_TRIG    (0 << 15)   // Edge-triggered mode (default)
#define IOAPIC_LEVEL_TRIG   (1 << 15)   // Level-triggered mode

#define IOAPIC_MASKED       (1 << 16)   // Interrupt masked (disabled)
#define IOAPIC_UNMASKED     (0 << 16)   // Interrupt unmasked (enabled)



void enable_ioapic_mode();

void ioapic_route_irq(uint8_t irq, uint8_t apic_id, uint8_t vector, uint32_t flags);

void ioapic_route_hardware_irq(uint8_t lapic_id, uint32_t flags);
void ioapic_route_syscall_irq(uint8_t lapic_id, uint32_t flags);

void ioapic_init();

