/*
Advanced Programmable Interrupt Controller
Reference:  https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/07_APIC.md
            https://wiki.osdev.org/APIC_Timer
*/

#include "../driver/ports.h"
#include "../driver/vga.h"
#include "../limine/limine.h"
#include "../lib/stdio.h"
#include "../util/util.h"
#include "../x86_64/idt/idt.h"

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

// MSR addresses
#define IA32_APIC_BASE_MSR 0x1B
#define APIC_TIMER_DIVIDE_CONFIG 0x3E0
#define APIC_TIMER_INITIAL_COUNT 0x380
#define APIC_TIMER_CURRENT_COUNT 0x390
#define APIC_TIMER_LVT 0x320
#define APIC_EOI 0xB0 // End of Interrupt register
#define APIC_ENABLE (1 << 11)

// APIC timer modes
#define APIC_TIMER_ONESHOT 0x0
#define APIC_TIMER_PERIODIC 0x20000

// Interrupt vector for the APIC timer
#define APIC_TIMER_VECTOR 32

// Function to read MSR
static inline uint64_t rdmsr(uint32_t msr) {
    uint32_t low, high;
    asm volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

// Function to write MSR
static inline void wrmsr(uint32_t msr, uint64_t value) {
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    asm volatile ("wrmsr" : : "a"(low), "d"(high), "c"(msr));
}

// Function to get the APIC base address
static uintptr_t get_apic_base() {
    uint64_t apic_base = rdmsr(IA32_APIC_BASE_MSR);
    return (apic_base & 0xFFFFF000); // Mask out the lower 12 bits
}

// Function to write to APIC register
static void apic_write(uintptr_t apic_base, uint32_t reg, uint32_t value) {
    *(volatile uint32_t*)(apic_base + reg) = value;
}

// Function to read from APIC register
static uint32_t apic_read(uintptr_t apic_base, uint32_t reg) {
    return *(volatile uint32_t*)(apic_base + reg);
}

// Function to send End of Interrupt (EOI) to the APIC
static void apic_send_eoi(uintptr_t apic_base) {
    apic_write(apic_base, APIC_EOI, 0);
}

// Function to start the APIC timer
void start_apic_timer(uintptr_t apic_base, uint32_t initial_count, uint8_t divide, uint32_t mode) {
    // Configure the divide configuration register
    apic_write(apic_base, APIC_TIMER_DIVIDE_CONFIG, divide);

    // Configure the LVT timer register
    apic_write(apic_base, APIC_TIMER_LVT, mode | APIC_TIMER_VECTOR);

    // Set the initial count
    apic_write(apic_base, APIC_TIMER_INITIAL_COUNT, initial_count);
}

// APIC Timer ISR
void apic_timer_isr(registers_t* regs) {
    (void)regs; // Suppress unused parameter warning

    // Print "tick"
    // print("tick\n");

    // Send EOI to the APIC
    apic_send_eoi(get_apic_base());
}

// Main function to initialize the APIC timer
void init_apic() {
    disable_pic();
    // Get the APIC base address
    uintptr_t apic_base = get_apic_base();

    // Install the APIC timer ISR
    interrupt_install_handler(0, &apic_timer_isr);

    // Start the APIC timer in periodic mode with a divide value of 16 and an initial count of 100000
    start_apic_timer(apic_base, 100000, 0x3, APIC_TIMER_PERIODIC);

    // Enable interrupts
    asm volatile ("sti");

    // Loop forever
    for (;;) {
        asm volatile ("hlt");
    }
}



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

