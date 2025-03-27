
/*
Advanced Programmable Interrupt Controller
Reference:  https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/07_APIC.md
            https://wiki.osdev.org/APIC
            https://wiki.osdev.org/IOAPIC
            https://wiki.osdev.org/LAPIC
            https://forum.osdev.org/viewtopic.php?p=107868#107868
            https://en.wikipedia.org/wiki/Advanced_Programmable_Interrupt_Controller
*/


#include "../../cpu/cpu.h"
#include "../../lib/stdio.h"
#include "../../driver/io/ports.h"
#include "ioapic.h"

#include "apic.h"

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100        // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800     // Enable APIC

#define APIC_SVR    0xF0            // Spurious Vector Register
#define APIC_EOI    0xB0            // End of Interrupt (EOI)
#define APIC_LVT    0x350           // Local Vector Table (LVT)
#define LAPIC_ID_REGISTER 0x20

#define LAPIC_ICRHI 0x310           // ICR High register
#define LAPIC_ICRLO 0x300           // ICR Low register

uint32_t LAPIC_BASE = 0xFEE00000;   // lapic base in general 0xFEE00000 but system may changed

extern bool has_apic();             // Defined in cpuid.c

// read 32 bit msr address and return 64-bit value
static inline uint64_t rdmsr(uint32_t msr) {
    uint32_t low, high;
    asm volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr)); // Stores the lower 32 bits from rax in low and higher 32 bits from rdx in high, and the MSR address in rcx
    return ((uint64_t)high << 32) | low;
}


// write 32 bit msr address with value
static inline void wrmsr(uint32_t msr, uint64_t value) {
    // splits the 64-bit value into two 32-bit values
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    asm volatile ("wrmsr" : : "c"(msr), "a"(low), "d"(high)); // Stores the lower 32 bits into rax from low and higher 32 bits from high in rdx, and the MSR address in rcx
}


void apic_write(uint32_t reg, uint32_t value) {
    *((volatile uint32_t *)(LAPIC_BASE + reg)) = value;
}

uint32_t apic_read(uint32_t reg) {
    return *((volatile uint32_t *)(LAPIC_BASE + reg));
}

void apic_send_eoi() {
    if (LAPIC_BASE) {
        *((volatile uint32_t*)(LAPIC_BASE + APIC_EOI)) = 0; // Send EOI to the LAPIC
    }
}

uint64_t get_lapic_base(){
    uint64_t msr = rdmsr(IA32_APIC_BASE_MSR);
    return msr & 0xFFFFF000;
}

uint32_t get_lapic_id() {
    return apic_read(LAPIC_ID_REGISTER) >> 24;
}



void lapic_send_ipi(uint8_t cpu_id, uint8_t vector) {
    uint32_t icr_hi = cpu_id << 24;               // Target CPU ID (Destination field)
    uint32_t icr_lo = (vector & 0xFF) | (0x4000); // Fixed delivery mode, vector number

    apic_write(LAPIC_ICRHI, icr_hi);
    apic_write(LAPIC_ICRLO, icr_lo);

    // Wait for delivery to complete
    while (*(volatile uint32_t*)LAPIC_ICRLO & (1 << 12));
}



void enable_apic() {
    LAPIC_BASE = rdmsr(0x1B) & 0xFFFFF000;  // Reading LAPIC_BASE

    if (!(rdmsr(0x1B) & (1 << 11))) {       // Check if APIC is enabled
        wrmsr(0x1B, LAPIC_BASE | (1 << 11)); // Enable APIC if disabled
    }
}


void init_apic_interrupt(){
    asm volatile("cli");

    LAPIC_BASE = get_lapic_base();

    enable_apic();
    apic_write(APIC_SVR, apic_read(APIC_SVR) | 0x100); // Enable APIC and set spurious vector
    enable_ioapic_mode();

    apic_send_eoi();

    asm volatile("sti");

    printf("Successfully APIC Interrupt enabled in CPU: %d.\n", get_lapic_id());
}








