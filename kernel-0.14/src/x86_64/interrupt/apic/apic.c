
/*
Advanced Programmable Interrupt Controller
Reference:  https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/07_APIC.md
            https://wiki.osdev.org/APIC
            https://wiki.osdev.org/IOAPIC
            https://wiki.osdev.org/LAPIC
            https://forum.osdev.org/viewtopic.php?p=107868#107868
            https://en.wikipedia.org/wiki/Advanced_Programmable_Interrupt_Controller
*/


#include "../../../cpu/cpu.h"
#include "../../../lib/stdio.h"
#include "../../../driver/io/ports.h"



#include "../../timer/tsc.h"

#include "apic.h"

// LAPIC Registers
#define LAPIC_ID_REGISTER 0x20
#define LAPIC_VERSION_REGISTER 0x30
#define LAPIC_TPR_REGISTER 0x80
#define LAPIC_APR_REGISTER 0x90
#define LAPIC_PPR_REGISTER 0xA0
#define LAPIC_EOI_REGISTER 0xB0
#define LAPIC_RRD_REGISTER 0xC0
#define LAPIC_LDR_REGISTER 0xD0
#define LAPIC_DFR_REGISTER 0xE0
#define LAPIC_SVR_REGISTER 0xF0             // Spurious Vector Register
#define LAPIC_ISRLO_REGISTER 0x100          // ISR Low register
#define LAPIC_ISRHI_REGISTER 0x170          // ISR High register

#define LAPIC_TMRLO_REGISTER 0x180          // TMR Low register
#define LAPIC_TMRHI_REGISTER 0x190          // TMR High register

#define LAPIC_IRRLO_REGISTER 0x200          // IRR Low register
#define LAPIC_IRRHI_REGISTER 0x280          // IRR High register
#define LAPIC_ESR_REGISTER 0x280
#define LAPIC_ICRLO_REGISTER 0x300           // ICR Low register
#define LAPIC_ICRHI_REGISTER 0x310           // ICR High register

#define LAPIC_LVT_TIMER_REGISTER 0x320
#define LAPIC_LVT_THERMAL_REGISTER 0x330
#define LAPIC_LVT_PERFMON_REGISTER 0x340
#define LAPIC_LVT_LINT0_REGISTER 0x350
#define LAPIC_LVT_LINT1_REGISTER 0x360
#define LAPIC_LVT_ERROR_REGISTER 0x370
#define LAPIC_TIMER_INITCNT_REGISTER 0x380
#define LAPIC_TIMER_CURRCNT_REGISTER 0x390
#define LAPIC_TIMER_DIV_REGISTER 0x3E0
#define LAPIC_TIMER_DIV_1 0x00000000

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100        // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800     // Enable APIC



uint32_t LAPIC_BASE = 0xFEE00000;   

extern bool has_apic();                     // Defined in cpuid.c


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
        *((volatile uint32_t*)(LAPIC_BASE + LAPIC_EOI_REGISTER)) = 0; // Send EOI to the LAPIC
    }
}

uint64_t get_lapic_base(){
    uint64_t msr = rdmsr(IA32_APIC_BASE_MSR);
    return msr & 0xFFFFF000;
}

uint32_t get_lapic_id() {
    return apic_read(LAPIC_ID_REGISTER) >> 24;
}

uint32_t get_lapic_version() {
    return apic_read(LAPIC_VERSION_REGISTER) & 0xFF; // Get the version of the LAPIC
}




/* Send an Inter-Processor Interrupt (IPI) to a specific CPU core.
 * cpu_id: The target CPU ID (LAPIC ID).
 * vector: The interrupt vector number to send.
 */
// This function sends an IPI to the specified CPU core using the LAPIC.
void lapic_send_ipi(uint8_t cpu_id, uint8_t vector) {
    uint32_t icr_hi = cpu_id << 24;               // Target CPU ID (Destination field)
    uint32_t icr_lo = (vector & 0xFF) | (0x4000); // Fixed delivery mode, vector number

    apic_write(LAPIC_ICRHI_REGISTER, icr_hi);
    apic_write(LAPIC_ICRLO_REGISTER, icr_lo);

    // Wait for delivery to complete
    // while (*(volatile uint32_t*)LAPIC_ICRLO_REGISTER & (1 << 12));
    while (apic_read(LAPIC_ICRLO_REGISTER) & (1 << 12));
}

bool is_apic_enabled() {
    uint64_t msr = rdmsr(IA32_APIC_BASE_MSR);
    return (msr & IA32_APIC_BASE_MSR_ENABLE) != 0; // Check if APIC is enabled
}


void enable_apic() {
    LAPIC_BASE = rdmsr(IA32_APIC_BASE_MSR) & 0xFFFFF000;  // Reading LAPIC_BASE

    uint32_t lapic_id = get_lapic_id(); // Get LAPIC ID

    if (!is_apic_enabled()) {       // Check if APIC is enabled
        wrmsr(IA32_APIC_BASE_MSR, LAPIC_BASE | IA32_APIC_BASE_MSR_ENABLE); // Enable APIC if disabled
    }
}


void disable_apic() {
    uint64_t msr = rdmsr(IA32_APIC_BASE_MSR);
    wrmsr(IA32_APIC_BASE_MSR, msr & ~IA32_APIC_BASE_MSR_ENABLE); // Disable APIC
}


// Initialize the LAPIC for interrupt handling for each core
void init_apic_interrupt(){
    asm volatile("cli");

    LAPIC_BASE = get_lapic_base();

    enable_apic();
    // apic_write(LAPIC_SVR_REGISTER, apic_read(LAPIC_SVR_REGISTER) | 0x100 | LAPIC_SVR_REGISTER); // Enable APIC and set spurious vector
    apic_write(LAPIC_SVR_REGISTER, (apic_read(LAPIC_SVR_REGISTER) & 0xFFFFFF00) | 0x100 | 0xFF);

    apic_send_eoi();

    asm volatile("sti");

    // printf(" [-] Successfully APIC Interrupt enabled in CPU: %d.\n", get_lapic_id());
    // printf(" [*] LAPIC Base: %x, LAPIC ID: %d, LAPIC Version: %d\n", get_lapic_base(), get_lapic_id(), get_lapic_version()); // Print LAPIC Base Address
}








