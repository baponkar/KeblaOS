
/*
Advanced Programmable Interrupt Controller
Reference:  https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/07_APIC.md
            https://wiki.osdev.org/APIC_Timer
*/

#include "../../lib/stdio.h"
#include "../../driver/io/ports.h"


#include "apic.h"


uint32_t LAPIC_BASE = 0xFEE00000;   // lapic base in general 0xFEE00000 but system may changed
#define APIC_SVR    0xF0            // Spurious Vector Register
#define APIC_EOI    0xB0            // End of Interrupt (EOI)
#define APIC_LVT    0x350           // Local Vector Table (LVT)
#define LAPIC_ID_REGISTER 0x20

#define LAPIC_ICRHI (LAPIC_BASE + 0x310) // ICR High register
#define LAPIC_ICRLO (LAPIC_BASE + 0x300) // ICR Low register


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
    uint64_t apic_base = rdmsr(0x1B) & 0xFFFFF000;  // IA32_APIC_BASE_MSR
    if (!(rdmsr(0x1B) & (1 << 11))) {
        wrmsr(0x1B, apic_base | (1 << 11));         // Enable APIC if disabled
    }
    LAPIC_BASE = apic_base;
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








