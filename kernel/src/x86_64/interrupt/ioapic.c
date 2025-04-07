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
#define IOAPICREDTBL    0x10    // R/W IOAPIC Redirection Table

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

/*
    Bit(s)	Field Name	        Description
    0-7	    Vector	            Interrupt vector number (IDT entry)
    8-10	Delivery Mode	    Specifies how the interrupt is delivered
    11	    Destination Mode	0 = Physical, 1 = Logical
    12	    Delivery Status	    0 = Idle, 1 = Send Pending (Read-Only)
    13	    Polarity	        0 = High Active, 1 = Low Active
    14	    Remote IRR	        0 = No interrupt pending, 1 = Interrupt waiting (Read-Only)
    15	    Trigger Mode	    0 = Edge Triggered, 1 = Level Triggered
    16	    Mask	            0 = Enabled, 1 = Disabled
*/

void ioapic_route_irq(uint8_t irq, uint8_t apic_id, uint8_t vector, uint32_t flags) {
    uint32_t reg_low  = IOAPICREDTBL + irq * 2;
    uint32_t reg_high = reg_low + 1;

    // High DWORD: Set destination APIC ID
    ioapic_write_reg(reg_high, (apic_id << 24));

    // Low DWORD: Vector + Flags (e.g., trigger mode, polarity)
    uint32_t low = vector | flags;
    ioapic_write_reg(reg_low, low);
}
 
// Map IRQ12 (PS/2 mouse) to interrupt vector 44 via IOAPIC
void configure_mouse_irq() {
    // Flags: Edge-triggered, Active High, Unmasked, Fixed delivery
    uint32_t flags = IOAPIC_EDGE_TRIG | IOAPIC_HIGH_ACTIVE | IOAPIC_FIXED | IOAPIC_UNMASKED;
    
    // Route IRQ12 to vector 44 for APIC ID 0 (bootstrap CPU)
    ioapic_route_irq(12, 0, 44, flags);
}

void configure_keyboard_irq(){
    // Flags: Edge-triggered, Active High, Unmasked, Fixed delivery
    uint32_t flags = IOAPIC_EDGE_TRIG | IOAPIC_HIGH_ACTIVE | IOAPIC_FIXED | IOAPIC_UNMASKED;
    
    // Route IRQ12 to vector 44 for APIC ID 0 (bootstrap CPU)
    ioapic_route_irq(1, 0, 33, flags);
}