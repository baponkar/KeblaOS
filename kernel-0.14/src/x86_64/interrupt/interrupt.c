/*

*/

#include "isr_manage.h"
#include "irq_manage.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"


#include "interrupt.h"


#define  TOTAL_INT_ENTRIES 256

extern void idt_flush(uint64_t);
extern void load_tss(uint64_t);

int_entry_t int_entries[TOTAL_INT_ENTRIES];
int_ptr_t   int_ptr;

void int_set_gate(uint8_t index, uint64_t offset, uint16_t selector, uint8_t attr){
    int_entry_t *entry = (int_entry_t *) &int_entries[index];

    entry->offset_1 = (uint16_t) (offset & 0xFFFF);             // set lower 16 bit
    entry->offset_2 = (uint16_t) (offset >> 16) & 0xFFFF;       // set 16 bit
    entry->offset_3 = (uint32_t) (offset >> 32) & 0xFFFFFFFF;   // set upper 32 bit

    entry->selector = selector;                                 // set 16 bit of selector
    //              |P|DPL|R|TYPE|
    // for x86_64 : |1|00 |0|1110| ==> 10001110 ==> 0x8E
    entry->type_attributes = attr;    // set 8 bit  of P(1 bit) + DPL(2 bit) + gate type(4 bit) + 0(1 bit)
    
    entry->ist = 0; // disabled ist i.e clear 3 bit of ist and 5 bit of reserved field 
    entry->zero = 0; // set top 32 bit to zero
}

void set_int_descriptor_table(){
    // Software Interrupts
    int_set_gate(0, (uint64_t)&isr0 ,  0x8, 0x8E);    // Division By Zero
    int_set_gate(1, (uint64_t)&isr1 ,  0x8, 0x8E);    // Debug
    int_set_gate(2, (uint64_t)&isr2 ,  0x8, 0x8E);    // Non Maskable Interrupt  
    int_set_gate(3, (uint64_t)&isr3 ,  0x8, 0x8E);    // Breakpoint 
    int_set_gate(4, (uint64_t)&isr4 ,  0x8, 0x8E);    // Into Detected Overflow
    int_set_gate(5, (uint64_t)&isr5 ,  0x8, 0x8E);    // Out of Bounds
    int_set_gate(6, (uint64_t)&isr6 ,  0x8, 0x8E);    // Invalid Opcode
    int_set_gate(7, (uint64_t)&isr7 ,  0x8, 0x8E);    // No Coprocessor
    int_set_gate(8, (uint64_t)&isr8 ,  0x8, 0x8E);    // Double fault (pushes an error code)
    int_set_gate(9, (uint64_t)&isr9 ,  0x8, 0x8E);    // Coprocessor Segment Overrun
    int_set_gate(10, (uint64_t)&isr10 , 0x8, 0x8E);    // Bad TSS (pushes an error code)
    int_set_gate(11, (uint64_t)&isr11 , 0x8, 0x8E);    // Segment not present (pushes an error code)
    int_set_gate(12, (uint64_t)&isr12 , 0x8, 0x8E);    // Stack fault (pushes an error code)
    int_set_gate(13, (uint64_t)&isr13 , 0x8, 0x8E);    // General protection fault (pushes an error code)
    int_set_gate(14, (uint64_t)&isr14 , 0x8, 0x8E);    // Page fault (pushes an error code)
    int_set_gate(15, (uint64_t)&isr15 , 0x8, 0x8E);    // Unknown Interrupt
    int_set_gate(16, (uint64_t)&isr16 , 0x8, 0x8E);    // Coprocessor Fault
    int_set_gate(17, (uint64_t)&isr17 , 0x8, 0x8E);    // Alignment Fault
    int_set_gate(18, (uint64_t)&isr18 , 0x8, 0x8E);    // Machine Check
    int_set_gate(19, (uint64_t)&isr19 , 0x8, 0x8E);    // SIMD (SSE/AVX) error
    int_set_gate(20, (uint64_t)&isr20 , 0x8, 0x8E);    // Reserved
    int_set_gate(21, (uint64_t)&isr21 , 0x8, 0x8E);    // Reserved
    int_set_gate(22, (uint64_t)&isr22 , 0x8, 0x8E);    // Reserved
    int_set_gate(23, (uint64_t)&isr23 , 0x8, 0x8E);    // Reserved
    int_set_gate(24, (uint64_t)&isr24 , 0x8, 0x8E);    // Reserved
    int_set_gate(25, (uint64_t)&isr25 , 0x8, 0x8E);    // Reserved
    int_set_gate(26, (uint64_t)&isr26 , 0x8, 0x8E);    // Reserved
    int_set_gate(27, (uint64_t)&isr27 , 0x8, 0x8E);    // Reserved
    int_set_gate(28, (uint64_t)&isr28 , 0x8, 0x8E);    // Reserved
    int_set_gate(29, (uint64_t)&isr29 , 0x8, 0x8E);    // Reserved
    int_set_gate(30, (uint64_t)&isr30 , 0x8, 0x8E);    // Reserved
    int_set_gate(31, (uint64_t)&isr31 , 0x8, 0x8E);    // Reserved

    // Hardware Interrupts
    int_set_gate(32, (uint64_t)&irq0, 0x08, 0x8E);    // Timer Interrupt, IRQ0
    int_set_gate(33, (uint64_t)&irq1, 0x08, 0x8E);    // Keyboard Interrupt, IRQ1
    // int_set_gate(34, (uint64_t)&irq2, 0x08, 0x8E);    // Cascade (for PIC chaining), IRQ2
    int_set_gate(35, (uint64_t)&irq3, 0x08, 0x8E);    // COM2 (Serial Port 2), IRQ3
    int_set_gate(36, (uint64_t)&irq4, 0x08, 0x8E);    // COM1 (Serial Port 1), IRQ4
    int_set_gate(37, (uint64_t)&irq5, 0x08, 0x8E);    // LPT2 (Parallel Port 2) or Sound Card, IRQ5
    int_set_gate(38, (uint64_t)&irq6, 0x08, 0x8E);    // Floppy Disk Controller, IRQ6
    int_set_gate(39, (uint64_t)&irq7, 0x08, 0x8E);    // LPT1 (Parallel Port 1) / Spurious IRQ, IRQ7
    int_set_gate(40, (uint64_t)&irq8, 0x08, 0x8E);    // Real-Time Clock (RTC), IRQ8
    int_set_gate(41, (uint64_t)&irq9, 0x08, 0x8E);    // ACPI / General system use, IRQ9
    int_set_gate(42, (uint64_t)&irq10, 0x08, 0x8E);   // Available (often used for SCSI or NIC), IRQ10
    int_set_gate(43, (uint64_t)&irq11, 0x08, 0x8E);   // Available (often used for PCI devices), IRQ11
    int_set_gate(44, (uint64_t)&irq12, 0x08, 0x8E);   // PS/2 Mouse, IRQ12
    int_set_gate(45, (uint64_t)&irq13, 0x08, 0x8E);   // FPU / Floating-Point Unit (Coprocessor), IRQ13
    int_set_gate(46, (uint64_t)&irq14, 0x08, 0x8E);   // Primary ATA Hard Disk Controller, IRQ14
    int_set_gate(47, (uint64_t)&irq15, 0x08, 0x8E);   // Secondary ATA Hard Disk Controller, IRQ15
    int_set_gate(48, (uint64_t)&irq16, 0x08, 0x8E);   // APIC Timer, IRQ16
    int_set_gate(49, (uint64_t)&irq17, 0x08, 0x8E);   // HPET Timer, IRQ17
    int_set_gate(50, (uint64_t)&irq18, 0x08, 0x8E);   // Available, IRQ18

    // System Calls
    int_set_gate(172, (uint64_t)&irq140, 0x08, 0xEE); // Print System Call, IRQ140
    int_set_gate(173, (uint64_t)&irq141, 0x08, 0xEE); // Read System Call, IRQ141
    int_set_gate(174, (uint64_t)&irq142, 0x08, 0xEE); // Exit System Call, IRQ142
}


// Initialize the Interrupt Descriptor Table (IDT) for bootstrap cpu core
void int_init(){

    asm volatile("cli");

    // for safety clearing memories
    memset((void *)&int_entries, 0, (sizeof(int_entry_t) *  TOTAL_INT_ENTRIES));
    set_int_descriptor_table();

    memset((void *) &int_ptr, 0, sizeof(int_ptr_t));
    uint16_t limit = (sizeof(int_entry_t) *  TOTAL_INT_ENTRIES) - 1;
    uint64_t base  = (uint64_t) &int_entries;
    int_ptr.limit = limit;
    int_ptr.base = base;

    idt_flush((uint64_t) &int_ptr);

    asm volatile("sti");

    printf("[Info] Successfully Bootstrap Interrupt Initialized.\n");
}

// Testing Interrupts and Debugging
void test_interrupt() {
    printf("Testing Interrupts\n");
    // asm volatile ("div %b0" :: "a"(0)); // Int no 0
    // asm volatile ("int $0x3");       // Breakpoint int no : 3
    // asm volatile ("int $0x0");       // Division By Zero, int no : 0
    // asm volatile ("int $0xE");       // Page Fault Request, int no: 14
    // asm volatile ("int $0xF");       // int no 15
    // asm volatile ("int $0x10");      // int no 16
    // asm volatile ("int $0x11");      // int no 17
    // asm volatile ("int $0x20");      // Interrupt Request, int no: 32, Timer Interrupt 
    // asm volatile ("int $0x21");      // Interrupt Request, int no : 33, Keyboard Interrupt
    // asm volatile ("int $0x2C");      // Interrupt Request, int no : 44, Mouse Interrupt
    // asm volatile ("int $0x22");      // Interrupt Request, int no: 34
    // asm volatile ("int $0x30");      // Interrupt Request, int no: 48
    
    asm volatile ("int $172");       // Interrupt Request for system call IRQ = 140, INT NO = 172
    asm volatile ("int $173");       // Interrupt Request for system call IRQ = 140, INT NO = 173
    asm volatile ("int $174");       // Interrupt Request for system call IRQ = 140, INT NO = 174
}
