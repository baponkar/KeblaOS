/*
    "Advanced Programmable Interrupt Controller(APIC)"
*/

#include "apic1.h"

// Defining some apic variables
#define LAPIC_BASE  0xFEE00000; // lapic base in general 0xFEE0000 but system may changed
#define APIC_SVR    0xF0  // Spurious Vector Register
#define APIC_EOI    0xB0  // End of Interrupt (EOI)
#define APIC_LVT    0x350  // Local Vector Table (LVT)
#define LAPIC_ID_REGISTER 0x20

#define IOAPIC_BASE   0xFEC00000
#define IOAPIC_REGSEL (IOAPIC_BASE)
#define IOAPIC_IOWIN  (IOAPIC_BASE + 0x10)

extern void apic_flush(uint64_t);

apic_entry_t apic_entries[256];
apic_ptr_t apic_ptr;

char* cpu_exception_messages[] = {
    "Division By Zero", // 0
    "Debug", // 1
    "Non Maskable Interrupt", // 2
    "Breakpoint", // 3
    "Into Detected Overflow", // 4
    "Out of Bounds", // 5
    "Invalid Opcode", // 6
    "No Coprocessor", // 7
    "Double fault (pushes an error code)", // 8
    "Coprocessor Segment Overrun", // 9
    "Bad TSS (pushes an error code)", // 10
    "Segment not present (pushes an error code)", // 11
    "Stack fault (pushes an error code)", // 12
    "General protection fault (pushes an error code)", // 13
    "Page fault (pushes an error code)", // 14
    "Unknown Interrupt", // 15
    "Coprocessor Fault", // 16
    "Alignment Fault", // 17
    "Machine Check",  // 18
    "SIMD (SSE/AVX) error", // 19
    "Reserved", // 20
    "Reserved", // 21
    "Reserved", // 22
    "Reserved", // 23
    "Reserved", // 24
    "Reserved", // 25
    "Reserved", // 26
    "Reserved", // 27
    "Reserved", // 28
    "Reserved", // 29
    "Reserved", // 30
    "Reserved"  // 31
};

// Write to a memory-mapped I/O address
static inline void mmio_write(uint32_t address, uint32_t value) {
    *((volatile uint32_t*)address) = value;
}


// Read from a memory-mapped I/O address
static inline uint32_t mmio_read(uint32_t address) {
    return *((volatile uint32_t*)address);
}


int has_apic() {
    uint32_t eax, ebx, ecx, edx;
    asm volatile ("cpuid"
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                  : "a"(1));
    return (edx & (1 << 9)) != 0; // Check APIC availability (Bit 9)
}


uint32_t get_lapic_id() {
    return mmio_read(LAPIC_BASE + LAPIC_ID_REGISTER) >> 24; // APIC ID is in bits 24-31
}


// read msr
uint64_t rdmsr(uint32_t msr) {
    uint32_t low, high;
    asm volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}


// write msr
void wrmsr(uint32_t msr, uint64_t value) {
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    asm volatile ("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

void enable_apic() {
    uint64_t apic_base = rdmsr(0x1B);    // Read APIC Base MSR (0x1B)
    LAPIC_BASE = apic_base & 0xFFFFFF00; // Extract base address
    apic_base |= (1 << 11);              // Set APIC Global Enable (Bit 11)
    wrmsr(0x1B, apic_base);              // Write back to MSR
}

void apic_send_eoi() {
    mmio_write(LAPIC_BASE + APIC_EOI, 0); // Acknowledge interrupt
}

void enable_ioapic_mode() {
    outb(0x22, 0x70);
    outb(0x23, 0x01);
}

static inline void ioapic_write(uint32_t reg, uint32_t value) {
    *((volatile uint32_t *)(IOAPIC_REGSEL)) = reg;
    *((volatile uint32_t *)(IOAPIC_IOWIN)) = value;
}

static inline uint32_t ioapic_read(uint32_t reg) {
    *((volatile uint32_t *)(IOAPIC_REGSEL)) = reg;
    return *((volatile uint32_t *)(IOAPIC_IOWIN));
}

void *interrupt_routines[16] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};


void init_apic(){
    if(has_apic){
        disable_interrupts();
        disable_pic();
        enable_apic();
        mmio_write(LAPIC_BASE + APIC_SVR, 0x1FF); // Enable APIC (bit 8) and set vector 0xFF
        apic_send_eoi();
        enable_ioapic_mode();

        apic_ptr.limit = (sizeof(apic_entry_t) * 256) - 1;
        apic_ptr.base  = (uint64_t) &apic_entries;
        memset((void *)&apic_entries, 0, (size_t) (sizeof(apic_entry_t) * 256)); // for safety clearing memories
        apic_flush((uint64_t) &apic_ptr);



        enable_interrupts();
    }else{
        printf("APIC is not supported!\n");
    }
}


