/*
Advanced Programmable Interrupt Controller
Reference:  https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/07_APIC.md
            https://wiki.osdev.org/APIC_Timer
*/

#include "../../driver/ports.h"
#include "../../lib/stdio.h"
#include "../../limine/limine.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../util/util.h"
#include "../../driver/keyboard.h"
#include "pic.h"

#include "apic.h"



#define PIC_COMMAND_MASTER 0x20
#define PIC_COMMAND_SLAVE 0xA0
#define PIC_DATA_MASTER 0x21
#define PIC_DATA_SLAVE 0xA1

#define ICW_1 0x11
#define ICW_2_M 0x20
#define ICW_2_S 0x28
#define ICW_3_M 0x04
#define ICW_3_S 0x02
#define ICW_4 0x01

// Function to disable the PIC
void disable_pic() {
    outb(PIC_COMMAND_MASTER, ICW_1);
    outb(PIC_COMMAND_SLAVE, ICW_1);
    outb(PIC_DATA_MASTER, ICW_2_M);
    outb(PIC_DATA_SLAVE, ICW_2_S);
    outb(PIC_DATA_MASTER, ICW_3_M);
    outb(PIC_DATA_SLAVE, ICW_3_S);
    outb(PIC_DATA_MASTER, ICW_4);
    outb(PIC_DATA_SLAVE, ICW_4);
    outb(PIC_DATA_MASTER, 0xFF);
    outb(PIC_DATA_SLAVE, 0xFF);
}

uint64_t LAPIC_BASE = 0xFEE00000; // lapic base in general 0xfee0000 but system may changed
#define APIC_SVR    0xFEE000F0 // Spurious Vector Register
#define APIC_EOI    0xFEE000B0  // End of Interrupt (EOI)
#define APIC_TIMER  0xFEE00320  // Timer Register
#define APIC_LVT    0xFEE00350  // Local Vector Table (LVT)
#define LAPIC_ID_REGISTER 0x20

#define IOAPIC_BASE   0xFEC00000
#define IOAPIC_REGSEL (IOAPIC_BASE)
#define IOAPIC_IOWIN  (IOAPIC_BASE + 0x10)

extern idt_entry_t idt_entries[256];
extern idt_ptr_t idt_ptr;

extern void idt_flush(uint64_t);

// Write to a memory-mapped I/O address
void mmio_write(uintptr_t address, uint64_t value) {
    *((volatile uint64_t*)address) = value;
}

// Read from a memory-mapped I/O address
uint64_t mmio_read(uintptr_t address) {
    return *((volatile uint64_t*)address);
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
    uint64_t apic_base = rdmsr(0x1B); // Read APIC Base MSR (0x1B)
    LAPIC_BASE = apic_base;
    apic_base |= (1 << 11);           // Set APIC Global Enable (Bit 11)
    wrmsr(0x1B, apic_base);           // Write back to MSR
}



void apic_send_eoi() {
    mmio_write(APIC_EOI, 0); // Acknowledge interrupt
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

void ioapic_remap_keyboard() {
    // Map IRQ1 (Keyboard) to APIC interrupt vector 0x21 (33)
    uint32_t redirection_entry = (0x21 & 0xFF) | (1 << 15); // Masked initially
    ioapic_write(0x12, redirection_entry);  // Set low 32 bits
    ioapic_write(0x13, 0);                  // Set high 32 bits (destination APIC ID)
    
    // Unmask the IRQ
    redirection_entry &= ~(1 << 15); // Unmask (bit 15 = 0)
    ioapic_write(0x12, redirection_entry);
}




void init_apic(){
    if(has_apic){
        disable_interrupts();
        disable_pic();
        enable_apic();
        mmio_write(APIC_SVR, 0x1FF); // Enable APIC (bit 8) and set vector 0xFF
        apic_send_eoi();
        enable_ioapic_mode();

        idt_ptr.limit = (sizeof(idt_entry_t) * 256) - 1;
        idt_ptr.base  = (uint64_t) &idt_entries;
        memset((void *)&idt_entries, 0, (size_t) (sizeof(idt_entry_t) * 256)); // for safety clearing memories
        idt_flush((uint64_t) &idt_ptr);

        isr_install();
        irq_install();

        ioapic_remap_keyboard();  // Remap keyboard IRQ1 to APIC
        

        enable_interrupts();
    }else{
        printf("APIC not supported!\n");
    }
}

