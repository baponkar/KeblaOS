
/*
Advanced Programmable Interrupt Controller
Reference:  https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/07_APIC.md
            https://wiki.osdev.org/APIC_Timer
*/
#include "../../cpu/cpu.h"
#include "../../lib/stdio.h"
#include "../../driver/io/ports.h"
#include "../../acpi/descriptor_table/madt.h"

#include "apic.h"





#define APIC_SVR    0xF0            // Spurious Vector Register
#define APIC_EOI    0xB0            // End of Interrupt (EOI)
#define APIC_LVT    0x350           // Local Vector Table (LVT)
#define LAPIC_ID_REGISTER 0x20

#define LAPIC_ICRHI (LAPIC_BASE + 0x310) // ICR High register
#define LAPIC_ICRLO (LAPIC_BASE + 0x300) // ICR Low register

uint32_t LAPIC_BASE = 0xFEE00000;   // lapic base in general 0xFEE00000 but system may changed
uint32_t IOAPIC_BASE = 0xFEC00000;  // Example IOAPIC base address
extern madt_t *madt_addr;           // To get IOAPIC base address

// Write to a memory-mapped I/O address
void mmio_write(uint32_t address, uint32_t value) {
    *((volatile uint32_t*)address) = value;
}


// Read from a memory-mapped I/O address
uint32_t mmio_read(uint32_t address) {
    return *((volatile uint32_t*)address);
}


int has_apic() {
    uint32_t eax, ebx, ecx, edx;
    asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    return (edx & (1 << 9)) != 0; // Check APIC availability (Bit 9)
}


uint32_t get_lapic_id() {
    return mmio_read(LAPIC_BASE + LAPIC_ID_REGISTER) >> 24; // APIC ID is in bits 24-31
}




void lapic_send_ipi(uint8_t cpu_id, uint8_t vector) {
    uint32_t icr_hi = cpu_id << 24;               // Target CPU ID (Destination field)
    uint32_t icr_lo = (vector & 0xFF) | (0x4000); // Fixed delivery mode, vector number

    // Write to ICR registers
    *(volatile uint32_t*)LAPIC_ICRHI = icr_hi;
    *(volatile uint32_t*)LAPIC_ICRLO = icr_lo;

    // Wait for delivery to complete
    while (*(volatile uint32_t*)LAPIC_ICRLO & (1 << 12));
}


// read msr
static inline uint64_t rdmsr(uint32_t msr) {
    uint32_t low, high;
    asm volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}


// write msr
static inline void wrmsr(uint32_t msr, uint64_t value) {
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    asm volatile ("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}


void enable_apic() {
    LAPIC_BASE = rdmsr(0x1B) & 0xFFFFF000;  // Reading LAPIC_BASE

    printf("LAPIC Base Address: %x\n", LAPIC_BASE);

    if(madt_addr){
        // IOAPIC_BASE = madt_addr->local_apic_address;
        printf("IOAPIC Base Address: %x\n", IOAPIC_BASE);
    }

    if (!(rdmsr(0x1B) & (1 << 11))) {       // Check if APIC is enabled
        wrmsr(0x1B, LAPIC_BASE | (1 << 11)); // Enable APIC if disabled
    }
}


void apic_send_eoi() {
    if (LAPIC_BASE) {
        *((volatile uint32_t*)(LAPIC_BASE + APIC_EOI)) = 0;
    }
}


void enable_ioapic_mode() {
    outb(0x22, 0x70);
    outb(0x23, 0x01);
}



void ioapic_route_irq(uint8_t irq, uint8_t apic_id, uint8_t vector) {
    // Calculate the redirection table register index offset
    uint32_t index_low  = 0x10 + irq * 2; // Low dword index
    uint32_t index_high = index_low + 1;   // High dword index

    // Write to IOAPIC redirection table registers using the IOAPIC base address
    mmio_write(IOAPIC_BASE + index_high * 4, (apic_id << 24));
    mmio_write(IOAPIC_BASE + index_low * 4, vector);
}




void init_apic_interrupt(){
    asm volatile("cli");

    enable_apic();
    // The spurious vector (lower 8 bits of SVR) determines what interrupt the LAPIC will send for spurious interrupts.
    mmio_write(LAPIC_BASE + APIC_SVR, mmio_read(LAPIC_BASE + APIC_SVR) | 0x100);
    apic_send_eoi();
    enable_ioapic_mode();

    asm volatile("sti");

    printf("Successfully APIC Interrupt enabled.\n");
}








