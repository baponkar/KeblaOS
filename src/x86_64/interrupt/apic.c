
/*
Advanced Programmable Interrupt Controller
Reference:  https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/07_APIC.md
            https://wiki.osdev.org/APIC_Timer
*/

#include "interrupt.h"
#include "../../limine/limine.h"
#include "../../lib/string.h"
#include "../../lib/stdio.h"
#include "../../driver/keyboard/keyboard.h"
#include "../../driver/io/ports.h"

#include "../../process/process.h"

#include "apic.h"


uint32_t LAPIC_BASE = 0xFEE00000; // lapic base in general 0xFEE00000 but system may changed
#define APIC_SVR    0xF0   // Spurious Vector Register
#define APIC_EOI    0xB0   // End of Interrupt (EOI)
#define APIC_LVT    0x350  // Local Vector Table (LVT)
#define LAPIC_ID_REGISTER 0x20

#define IOAPIC_BASE   0xFEC00000
#define IOAPIC_REGSEL (IOAPIC_BASE)
#define IOAPIC_IOWIN  (IOAPIC_BASE + 0x10)

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
    asm volatile ("cpuid"
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                  : "a"(1));
    return (edx & (1 << 9)) != 0; // Check APIC availability (Bit 9)
}

uint32_t get_lapic_id() {
    return mmio_read(LAPIC_BASE + LAPIC_ID_REGISTER) >> 24; // APIC ID is in bits 24-31
}

void lapic_send_ipi(uint8_t cpu_id, uint8_t vector) {
    uint32_t icr_hi = cpu_id << 24; // Target CPU ID (Destination field)
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
        wrmsr(0x1B, apic_base | (1 << 11)); // Enable APIC if disabled
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

void ioapic_remap_timer() {
    // Map IRQ0 (PIT Timer) to APIC interrupt vector 0x20 (32)
    uint32_t redirection_entry = (0x20 & 0xFF) | (1 << 15); // Masked initially
    ioapic_write(0x10, redirection_entry);  // Set low 32 bits
    ioapic_write(0x11, 0);                  // Set high 32 bits (destination APIC ID)

    // Unmask the IRQ
    redirection_entry &= ~(1 << 15); // Unmask (bit 15 = 0)
    ioapic_write(0x10, redirection_entry);
}


// Installing ISR or CPU Exception into IDT
void cpu_exception_install(){
   int_set_gate(  0, (uint64_t)&apic_isr0 ,  0x8, 0x8E);  // selector = 0x08 = 0b1000, 64-bit Interrupt Gate => attr = 0x8E = 1 0 00 1110, (p=0b1,0b0, dpl=0b00, gate type=0b1110)
   int_set_gate(  1, (uint64_t)&apic_isr1 ,  0x8, 0x8E);
   int_set_gate(  2, (uint64_t)&apic_isr2 ,  0x8, 0x8E);  // selector value is 1000 because GDT code segment index is 1
   int_set_gate(  3, (uint64_t)&apic_isr3 ,  0x8, 0x8E);  // selector = index + table_to_use + privilege
   int_set_gate(  4, (uint64_t)&apic_isr4 ,  0x8, 0x8E);  // selector  = 1<<3(index 1) + 0<<2(TI for GDT 0) + 0<<1(for ring 0) => 1000 + 000 + 00 = 1000 = 0x08
   int_set_gate(  5, (uint64_t)&apic_isr5 ,  0x8, 0x8E);
   int_set_gate(  6, (uint64_t)&apic_isr6 ,  0x8, 0x8E);
   int_set_gate(  7, (uint64_t)&apic_isr7 ,  0x8, 0x8E);
   int_set_gate(  8, (uint64_t)&apic_isr8 ,  0x8, 0x8E);
   int_set_gate(  9, (uint64_t)&apic_isr9 ,  0x8, 0x8E);
   int_set_gate( 10, (uint64_t)&apic_isr10 , 0x8, 0x8E);
   int_set_gate( 11, (uint64_t)&apic_isr11 , 0x8, 0x8E);
   int_set_gate( 12, (uint64_t)&apic_isr12 , 0x8, 0x8E);
   int_set_gate( 13, (uint64_t)&apic_isr13 , 0x8, 0x8E);
   int_set_gate( 14, (uint64_t)&apic_isr14 , 0x8, 0x8E); // paging
   int_set_gate( 15, (uint64_t)&apic_isr15 , 0x8, 0x8E);

   int_set_gate( 16, (uint64_t)&apic_isr16 , 0x8, 0x8E);
   int_set_gate( 17, (uint64_t)&apic_isr17 , 0x8, 0x8E);
   int_set_gate( 18, (uint64_t)&apic_isr18 , 0x8, 0x8E);
   int_set_gate( 19, (uint64_t)&apic_isr19 , 0x8, 0x8E);
   int_set_gate( 20, (uint64_t)&apic_isr20 , 0x8, 0x8E);
   int_set_gate( 21, (uint64_t)&apic_isr21 , 0x8, 0x8E);
   int_set_gate( 22, (uint64_t)&apic_isr22 , 0x8, 0x8E);
   int_set_gate( 23, (uint64_t)&apic_isr23 , 0x8, 0x8E);
   int_set_gate( 24, (uint64_t)&apic_isr24 , 0x8, 0x8E);
   int_set_gate( 25, (uint64_t)&apic_isr25 , 0x8, 0x8E);
   int_set_gate( 26, (uint64_t)&apic_isr26 , 0x8, 0x8E);
   int_set_gate( 27, (uint64_t)&apic_isr27 , 0x8, 0x8E);
   int_set_gate( 28, (uint64_t)&apic_isr28 , 0x8, 0x8E);
   int_set_gate( 29, (uint64_t)&apic_isr29 , 0x8, 0x8E);
   int_set_gate( 30, (uint64_t)&apic_isr30 , 0x8, 0x8E);
   int_set_gate( 31, (uint64_t)&apic_isr31 , 0x8, 0x8E);
}

// Installing IRQ into IDT
void apic_irq_install(){
    int_set_gate(32, (uint64_t)&apic_irq0, 0x08, 0x8E);  // PIT Timer Interrupt
    int_set_gate(33, (uint64_t)&apic_irq1, 0x08, 0x8E);  // Keyboard Interrupt
    int_set_gate(34, (uint64_t)&apic_irq2, 0x08, 0x8E);  // Cascade (for PIC chaining)
    int_set_gate(35, (uint64_t)&apic_irq3, 0x08, 0x8E);  // COM2 (Serial Port 2)
    int_set_gate(36, (uint64_t)&apic_irq4, 0x08, 0x8E);  // COM1 (Serial Port 1)
    int_set_gate(37, (uint64_t)&apic_irq5, 0x08, 0x8E);  // LPT2 (Parallel Port 2) or Sound Card
    int_set_gate(38, (uint64_t)&apic_irq6, 0x08, 0x8E);  // Floppy Disk Controller
    int_set_gate(39, (uint64_t)&apic_irq7, 0x08, 0x8E);  // LPT1 (Parallel Port 1) / Spurious IRQ
    int_set_gate(40, (uint64_t)&apic_irq8, 0x08, 0x8E);  // Real-Time Clock (RTC)
    int_set_gate(41, (uint64_t)&apic_irq9, 0x08, 0x8E);  // ACPI / General system use
    int_set_gate(42, (uint64_t)&apic_irq10, 0x08, 0x8E); // Available (often used for SCSI or NIC)
    int_set_gate(43, (uint64_t)&apic_irq11, 0x08, 0x8E); // Available (often used for PCI devices)
    int_set_gate(44, (uint64_t)&apic_irq12, 0x08, 0x8E); // PS/2 Mouse
    int_set_gate(45, (uint64_t)&apic_irq13, 0x08, 0x8E); // FPU / Floating-Point Unit (Coprocessor)
    int_set_gate(46, (uint64_t)&apic_irq14, 0x08, 0x8E); // Primary ATA Hard Disk Controller
    int_set_gate(47, (uint64_t)&apic_irq15, 0x08, 0x8E); // Secondary ATA Hard Disk Controller
    int_set_gate(48, (uint64_t)&apic_irq16, 0x08, 0x8E); // APIC Timer
}


void apic_irq_handler(registers_t *regs)
{   
    
    /* This is a blank function pointer */
    void (*handler)(registers_t *r);
    
    /* Find out if we have a custom handler to run for this
    *  IRQ, and then finally, run it */
    handler = interrupt_routines[regs->int_no - 32];


    if (handler)
    {
        handler(regs);
    }

    apic_send_eoi();
}



void init_apic_interrupt(){
    disable_interrupts();
    disable_pic();
    enable_apic();

    mmio_write(LAPIC_BASE + APIC_SVR, mmio_read(LAPIC_BASE + APIC_SVR) | 0x100);
    // mmio_write(LAPIC_BASE + APIC_SVR, 0x1FF); // Enable APIC (bit 8) and set vector 0xFF

    apic_send_eoi();
    enable_ioapic_mode();

    int_ptr.limit = (sizeof(int_entry_t) * 256) - 1;
    int_ptr.base  = (uint64_t) &int_entries;
    memset((void *)&int_entries, 0, (size_t) (sizeof(int_entry_t) * 256)); // for safety clearing memories
    interrupt_flush((uint64_t) &int_ptr);

    cpu_exception_install();
    apic_irq_install();

    ioapic_remap_keyboard();  // Remap keyboard IRQ1 to APIC

    enable_interrupts();

    printf("Successfully APIC Interrupt enabled.\n");
}



// This gets called from our ASM interrupt handler stub.
void apic_isr_handler(registers_t *regs)
{
    printf("Inside of APIC_ISR_HANDLER\n");
    if(regs->int_no == 128){
        printf("Received Interrupt(ISR) : %d\n", regs->int_no);
        // syscall_handler(&regs);
        return;
    }else if(regs->int_no == 177){
        printf("Received Interrupt(ISR) : %d\n", regs->int_no);
        // syscall_handler(&regs);
        return;
    }else if (regs->int_no == 14) {
        // page_fault_handler(regs);
        return;
    }else if(regs->int_no == 13){
        // print("General Protection Fault\n");
        // debug_error_code(regs->err_code);
        gpf_handler(regs);
        return;
    }else if(regs->int_no < 32){
        printf("Received Interrupt(ISR) : %d\n%s\nError Code : %d\nSystem Halted!\n", 
            regs->int_no, exception_messages[regs->int_no], regs->err_code);
        // debug_error_code(regs->err_code);
        halt_kernel();
    }else{
        printf("Received Interrupt(ISR) : %d\n", regs->int_no);
        return;
    }
}





