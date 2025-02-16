/*
Advanced Programmable Interrupt Controller
Reference:  https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/07_APIC.md
            https://wiki.osdev.org/APIC_Timer
*/

#include "../../driver/io/ports.h"
#include "../../lib/stdio.h"
#include "../../limine/limine.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../driver/keyboard/keyboard.h"
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

uint32_t LAPIC_BASE = 0; // lapic base in general 0xFEE0000 but system may changed
#define APIC_SVR    0xF0  // Spurious Vector Register
#define APIC_EOI    0xB0  // End of Interrupt (EOI)
#define APIC_LVT    0x350  // Local Vector Table (LVT)
#define LAPIC_ID_REGISTER 0x20

#define IOAPIC_BASE   0xFEC00000
#define IOAPIC_REGSEL (IOAPIC_BASE)
#define IOAPIC_IOWIN  (IOAPIC_BASE + 0x10)

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr128();
extern void isr177();

extern idt_entry_t idt_entries[256];
extern idt_ptr_t idt_ptr;

extern void idt_flush(uint64_t);

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


void cpu_exception_install(){
   idt_set_gate(  0, (uint64_t)&isr0 ,  0x8, 0x8E);  // selector = 0x08 = 0b1000, 64-bit Interrupt Gate => attr = 0x8E = 1 0 00 1110, (p=0b1,0b0, dpl=0b00, gate type=0b1110)
   idt_set_gate(  1, (uint64_t)&isr1 ,  0x8, 0x8E);
   idt_set_gate(  2, (uint64_t)&isr2 ,  0x8, 0x8E);  // selector value is 1000 because GDT code segment index is 1
   idt_set_gate(  3, (uint64_t)&isr3 ,  0x8, 0x8E);  // selector = index + table_to_use + privilege
   idt_set_gate(  4, (uint64_t)&isr4 ,  0x8, 0x8E);  // selector  = 1<<3(index 1) + 0<<2(TI for GDT 0) + 0<<1(for ring 0) => 1000 + 000 + 00 = 1000 = 0x08
   idt_set_gate(  5, (uint64_t)&isr5 ,  0x8, 0x8E);
   idt_set_gate(  6, (uint64_t)&isr6 ,  0x8, 0x8E);
   idt_set_gate(  7, (uint64_t)&isr7 ,  0x8, 0x8E);
   idt_set_gate(  8, (uint64_t)&isr8 ,  0x8, 0x8E);
   idt_set_gate(  9, (uint64_t)&isr9 ,  0x8, 0x8E);
   idt_set_gate( 10, (uint64_t)&isr10 , 0x8, 0x8E);
   idt_set_gate( 11, (uint64_t)&isr11 , 0x8, 0x8E);
   idt_set_gate( 12, (uint64_t)&isr12 , 0x8, 0x8E);
   idt_set_gate( 13, (uint64_t)&isr13 , 0x8, 0x8E);
   idt_set_gate( 14, (uint64_t)&isr14 , 0x8, 0x8E); // paging
   idt_set_gate( 15, (uint64_t)&isr15 , 0x8, 0x8E);

   idt_set_gate( 16, (uint64_t)&isr16 , 0x8, 0x8E);
   idt_set_gate( 17, (uint64_t)&isr17 , 0x8, 0x8E);
   idt_set_gate( 18, (uint64_t)&isr18 , 0x8, 0x8E);
   idt_set_gate( 19, (uint64_t)&isr19 , 0x8, 0x8E);
   idt_set_gate( 20, (uint64_t)&isr20 , 0x8, 0x8E);
   idt_set_gate( 21, (uint64_t)&isr21 , 0x8, 0x8E);
   idt_set_gate( 22, (uint64_t)&isr22 , 0x8, 0x8E);
   idt_set_gate( 23, (uint64_t)&isr23 , 0x8, 0x8E);
   idt_set_gate( 24, (uint64_t)&isr24 , 0x8, 0x8E);
   idt_set_gate( 25, (uint64_t)&isr25 , 0x8, 0x8E);
   idt_set_gate( 26, (uint64_t)&isr26 , 0x8, 0x8E);
   idt_set_gate( 27, (uint64_t)&isr27 , 0x8, 0x8E);
   idt_set_gate( 28, (uint64_t)&isr28 , 0x8, 0x8E);
   idt_set_gate( 29, (uint64_t)&isr29 , 0x8, 0x8E);
   idt_set_gate( 30, (uint64_t)&isr30 , 0x8, 0x8E);
   idt_set_gate( 31, (uint64_t)&isr31 , 0x8, 0x8E);
}

void apic_irq_install(){
    idt_set_gate(32, (uint64_t)&irq0, 0x08, 0x8E);  // Timer Interrupt
    idt_set_gate(33, (uint64_t)&irq1, 0x08, 0x8E);  // Keyboard Interrupt
    idt_set_gate(34, (uint64_t)&irq2, 0x08, 0x8E);  // Cascade (for PIC chaining)
    idt_set_gate(35, (uint64_t)&irq3, 0x08, 0x8E);  // COM2 (Serial Port 2)
    idt_set_gate(36, (uint64_t)&irq4, 0x08, 0x8E);  // COM1 (Serial Port 1)
    idt_set_gate(37, (uint64_t)&irq5, 0x08, 0x8E);  // LPT2 (Parallel Port 2) or Sound Card
    idt_set_gate(38, (uint64_t)&irq6, 0x08, 0x8E);  // Floppy Disk Controller
    idt_set_gate(39, (uint64_t)&irq7, 0x08, 0x8E);  // LPT1 (Parallel Port 1) / Spurious IRQ
    idt_set_gate(40, (uint64_t)&irq8, 0x08, 0x8E);  // Real-Time Clock (RTC)
    idt_set_gate(41, (uint64_t)&irq9, 0x08, 0x8E);  // ACPI / General system use
    idt_set_gate(42, (uint64_t)&irq10, 0x08, 0x8E); // Available (often used for SCSI or NIC)
    idt_set_gate(43, (uint64_t)&irq11, 0x08, 0x8E); // Available (often used for PCI devices)
    idt_set_gate(44, (uint64_t)&irq12, 0x08, 0x8E); // PS/2 Mouse
    idt_set_gate(45, (uint64_t)&irq13, 0x08, 0x8E); // FPU / Floating-Point Unit (Coprocessor)
    idt_set_gate(46, (uint64_t)&irq14, 0x08, 0x8E); // Primary ATA Hard Disk Controller
    idt_set_gate(47, (uint64_t)&irq15, 0x08, 0x8E); // Secondary ATA Hard Disk Controller
    idt_set_gate(48, (uint64_t)&irq15, 0x08, 0x8E); // Secondary ATA Hard Disk Controller
}

/* This array is actually an array of function pointers. We use
*  this to handle custom Interrupt handlers for a given Interrupt */
void *apic_interrupt_routines[16] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};



/* This installs a custom Interrupt handler for the given Interrupt */
void apic_interrupt_install_handler(int irq_no, void (*handler)(registers_t *r))
{
    apic_interrupt_routines[irq_no] = handler;
}


/* This clears the handler for a given Interrupt */
void apic_interrupt_uninstall_handler(int int_no)
{
    apic_interrupt_routines[int_no] = 0;
}

void apic_irq_handler(registers_t *regs)
{
    /* This is a blank function pointer */
    void (*handler)(registers_t *r);
    
    /* Find out if we have a custom handler to run for this
    *  IRQ, and then finally, run it */
    handler = apic_interrupt_routines[regs->int_no - 32];

    if (handler)
    {
        handler(regs);
    }

    apic_send_eoi();
}



void init_apic(){
    if(has_apic){
        disable_interrupts();
        disable_pic();
        enable_apic();

        mmio_write(LAPIC_BASE + APIC_SVR, mmio_read(LAPIC_BASE + APIC_SVR) | 0x100);
        // mmio_write(LAPIC_BASE + APIC_SVR, 0x1FF); // Enable APIC (bit 8) and set vector 0xFF

        apic_send_eoi();
        enable_ioapic_mode();

        idt_ptr.limit = (sizeof(idt_entry_t) * 256) - 1;
        idt_ptr.base  = (uint64_t) &idt_entries;
        memset((void *)&idt_entries, 0, (size_t) (sizeof(idt_entry_t) * 256)); // for safety clearing memories
        idt_flush((uint64_t) &idt_ptr);

        cpu_exception_install();
        apic_irq_install();

        ioapic_remap_keyboard();  // Remap keyboard IRQ1 to APIC

        enable_interrupts();
    }else{
        printf("APIC not supported!\n");
    }
}




