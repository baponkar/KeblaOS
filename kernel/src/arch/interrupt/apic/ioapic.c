/*
IOAPIC (I/O Advanced Programmable Interrupt Controller) is a part of the Intel APIC architecture. 
It is used to route interrupts from I/O devices to the CPU. 
It is a separate chip on the motherboard and is connected to the CPU through the system bus

https://wiki.osdev.org/IOAPIC
*/

#include "../../../lib/stdio.h"
#include "../../../driver/io/ports.h"
#include "../irq_manage.h"
#include "ioapic.h"


// Offsets for IOAPIC registers
#define IOAPIC_REGSEL   0x00
#define IOAPIC_WINDOW   0x10

// The above registers used to detect below registers
#define IOAPICID        0x00    // E/W
#define IOAPICVER       0x01    // R/O
#define IOAPICARB       0x02    // R/O
#define IOAPICREDTBL    0x10    // R/W IOAPIC Redirection Table

// Global IOAPIC base address (set in madt.c)
uint32_t ioapic_addr = 0xFEC00000; // Default address for IOAPIC;   

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


 
void ioapic_route_irq(uint8_t irq_no, uint8_t apic_id, uint8_t vector_no, uint32_t flags) {
    uint32_t reg_low  = IOAPICREDTBL + irq_no * 2;
    uint32_t reg_high = reg_low + 1;

    // --- HIGH DWORD: Set destination APIC ID ---
    uint32_t high = ioapic_read_reg(reg_high);          // Read existing
    high &= 0x00FFFFFF;                                 // Clear bits 24-31 (destination field)
    high |= ((uint32_t)apic_id) << 24;                  // Set new APIC ID
    ioapic_write_reg(reg_high, high);                   // Write back

    // --- LOW DWORD: Set vector number and flags ---
    uint32_t low = vector_no | flags;
    ioapic_write_reg(reg_low, low);
}


void ioapic_route_all_irq(uint8_t lapic_id, uint32_t flags) {
    ioapic_route_irq(0, lapic_id, 32, flags);      // Route IRQ 1 to current LAPIC ID with vector 32 : Timer interrupt
    ioapic_route_irq(1, lapic_id, 33, flags);      // Route IRQ 1 to current LAPIC ID with vector 33 : Keyboard interrupt
    // ioapic_route_irq(2, lapic_id, 34, flags);      // Route IRQ 2 to current LAPIC ID with vector 34 : Mouse interrupt
    ioapic_route_irq(3, lapic_id, 35, flags);      // Route IRQ 3 to current LAPIC ID with vector 35 : Cascade interrupt
    ioapic_route_irq(4, lapic_id, 36, flags);      // Route IRQ 4 to current LAPIC ID with vector 36 : Serial Port 1 interrupt
    ioapic_route_irq(5, lapic_id, 37, flags);      // Route IRQ 5 to current LAPIC ID with vector 37 : Serial Port 2 interrupt
    ioapic_route_irq(6, lapic_id, 38, flags);      // Route IRQ 6 to current LAPIC ID with vector 38 : Floppy Disk interrupt
    ioapic_route_irq(7, lapic_id, 39, flags);      // Route IRQ 7 to current LAPIC ID with vector 39 : Parallel Port 1 interrupt
    ioapic_route_irq(8, lapic_id, 40, flags);      // Route IRQ 8 to current LAPIC ID with vector 40 : Real-Time Clock interrupt
    ioapic_route_irq(9, lapic_id, 41, flags);      // Route IRQ 9 to current LAPIC ID with vector 41 : ACPI interrupt
    ioapic_route_irq(10,lapic_id, 42, flags);     // Route IRQ 10 to current LAPIC ID with vector 42 : Available interrupt
    ioapic_route_irq(11,lapic_id, 43, flags);     // Route IRQ 11 to current LAPIC ID with vector 43 : Available interrupt
    ioapic_route_irq(12,lapic_id, 44, flags);     // Route IRQ 12 to current LAPIC ID with vector 44 : PS/2 Mouse interrupt
    ioapic_route_irq(13,lapic_id, 45, flags);     // Route IRQ 13 to current LAPIC ID with vector 45 : FPU interrupt
    ioapic_route_irq(14,lapic_id, 46, flags);     // Route IRQ 14 to current LAPIC ID with vector 46 : Primary ATA Hard Disk interrupt
    ioapic_route_irq(15,lapic_id, 47, flags);     // Route IRQ 15 to current LAPIC ID with vector 47 : Secondary ATA Hard Disk interrupt
    ioapic_route_irq(16,lapic_id, 48, flags);     // Route IRQ 16 to current LAPIC ID with vector 48 : APIC Timer interrupt
    ioapic_route_irq(17,lapic_id, 49, flags);     // Route IRQ 17 to current LAPIC ID with vector 49 : HPET Timer interrupt
    ioapic_route_irq(18,lapic_id, 50, flags);     // Route IRQ 18 to current LAPIC ID with vector 50 : Available interrupt

    
    // Custom System Calls(51-256)
    ioapic_route_irq(19, lapic_id, 51, flags);   // Route IRQ 19 to current LAPIC ID with vector 51
    ioapic_route_irq(20, lapic_id, 52, flags);   // Route IRQ 20 to current LAPIC ID with vector 52 
    ioapic_route_irq(21, lapic_id, 53, flags);   // Route IRQ 21 to current LAPIC ID with vector 53 
    ioapic_route_irq(22, lapic_id, 54, flags);   // Route IRQ 22 to current LAPIC ID with vector 54 
    ioapic_route_irq(23, lapic_id, 55, flags);   // Route IRQ 23 to current LAPIC ID with vector 55
    ioapic_route_irq(24, lapic_id, 56, flags);   // Route IRQ 24 to current LAPIC ID with vector 56
    ioapic_route_irq(25, lapic_id, 57, flags);   // Route IRQ 25 to current LAPIC ID with vector 57
    ioapic_route_irq(26, lapic_id, 58, flags);   // Route IRQ 26 to current LAPIC ID with vector 58
    ioapic_route_irq(27, lapic_id, 59, flags);   // Route IRQ 27 to current LAPIC ID with vector 59
    ioapic_route_irq(28, lapic_id, 60, flags);   // Route IRQ 28 to current LAPIC ID with vector 60
    ioapic_route_irq(29, lapic_id, 61, flags);   // Route IRQ 29 to current LAPIC ID with vector 61
    ioapic_route_irq(30, lapic_id, 62, flags);   // Route IRQ 30 to current LAPIC ID with vector 62
    ioapic_route_irq(31, lapic_id, 63, flags);   // Route IRQ 31 to current LAPIC ID with vector 63
    ioapic_route_irq(32, lapic_id, 64, flags);   // Route IRQ 32 to current LAPIC ID with vector 64
    ioapic_route_irq(33, lapic_id, 65, flags);   // Route IRQ 33 to current LAPIC ID with vector 65
    ioapic_route_irq(34, lapic_id, 66, flags);   // Route IRQ 34 to current LAPIC ID with vector 66
    ioapic_route_irq(35, lapic_id, 67, flags);   // Route IRQ 35 to current LAPIC ID with vector 67
    ioapic_route_irq(36, lapic_id, 68, flags);   // Route IRQ 36 to current LAPIC ID with vector 68
    ioapic_route_irq(37, lapic_id, 69, flags);   // Route IRQ 37 to current LAPIC ID with vector 69
    ioapic_route_irq(38, lapic_id, 70, flags);   // Route IRQ 38 to current LAPIC ID with vector 70
    ioapic_route_irq(39, lapic_id, 71, flags);   // Route IRQ 39 to current LAPIC ID with vector 71
    ioapic_route_irq(40, lapic_id, 72, flags);   // Route IRQ 40 to current LAPIC ID with vector 72
    ioapic_route_irq(41, lapic_id, 73, flags);   // Route IRQ 41 to current LAPIC ID with vector 73
    ioapic_route_irq(42, lapic_id, 74, flags);   // Route IRQ 42 to current LAPIC ID with vector 74
    ioapic_route_irq(43, lapic_id, 75, flags);   // Route IRQ 43 to current LAPIC ID with vector 75
    ioapic_route_irq(44, lapic_id, 76, flags);   // Route IRQ 44 to current LAPIC ID with vector 76
    ioapic_route_irq(45, lapic_id, 77, flags);   // Route IRQ 45 to current LAPIC ID with vector 77
    ioapic_route_irq(46, lapic_id, 78, flags);   // Route IRQ 46 to current LAPIC ID with vector 78
    ioapic_route_irq(47, lapic_id, 79, flags);   // Route IRQ 47 to current LAPIC ID with vector 79
    ioapic_route_irq(48, lapic_id, 80, flags);   // Route IRQ 48 to current LAPIC ID with vector 80
    ioapic_route_irq(49, lapic_id, 81, flags);   // Route IRQ 49 to current LAPIC ID with vector 81
    ioapic_route_irq(50, lapic_id, 82, flags);   // Route IRQ 50 to current LAPIC ID with vector 82
    ioapic_route_irq(51, lapic_id, 83, flags);   // Route IRQ 51 to current LAPIC ID with vector 83
    ioapic_route_irq(52, lapic_id, 84, flags);   // Route IRQ 52 to current LAPIC ID with vector 84
    ioapic_route_irq(53, lapic_id, 85, flags);   // Route IRQ 53 to current LAPIC ID with vector 85
    ioapic_route_irq(54, lapic_id, 86, flags);   // Route IRQ 54 to current LAPIC ID with vector 86
    ioapic_route_irq(55, lapic_id, 87, flags);   // Route IRQ 55 to current LAPIC ID with vector 87
    ioapic_route_irq(56, lapic_id, 88, flags);   // Route IRQ 56 to current LAPIC ID with vector 88


    ioapic_route_irq(57, lapic_id, 89, flags);   // Route IRQ 89 to current LAPIC ID with vector 172 : Print System Call
    ioapic_route_irq(58, lapic_id, 90, flags);   // Route IRQ 90 to current LAPIC ID with vector 173 : Read System Call
    ioapic_route_irq(59, lapic_id, 91, flags);   // Route IRQ 91 to current LAPIC ID with vector 174 : Exit System Call
    ioapic_route_irq(60, lapic_id, 92, flags);   // Route IRQ 92 to current LAPIC ID with vector 174 : Exit System Call
    ioapic_route_irq(61, lapic_id, 93, flags);   // Route IRQ 93 to current LAPIC ID with vector 175 : Alloc System Call
    ioapic_route_irq(62, lapic_id, 94, flags);   // Route IRQ 94 to current LAPIC ID with vector 176 : Free System Call

    printf(" [-] IOAPIC Hardware IRQs routed to LAPIC ID %d\n", lapic_id);
}





void ioapic_init() {
    // Read IOAPIC ID and version
    uint32_t ioapic_id = ioapic_read_reg(IOAPICID) >> 24; // Get the IOAPIC ID (bits 24-31)
    uint32_t ioapic_ver = ioapic_read_reg(IOAPICVER) & 0xFF; // Get the IOAPIC version (bits 0-7)

    printf("IOAPIC ID: %u, Version: %u\n", ioapic_id, ioapic_ver);
}




