/*
8259 PIC (Programmable Interrupt Controller) driver for x86_64 architecture.

https://wiki.osdev.org/8259_PIC

*/

#include "../isr_manage.h"
#include "../irq_manage.h"

#include "../../../lib/stdio.h"
#include "../../../lib/string.h"

#include "pic.h"

#include "pic_interrupt.h"


#define  TOTAL_INT_ENTRIES 256



extern void idt_flush(uint64_t);
extern void load_tss(uint64_t);

int_entry_t int_entries[TOTAL_INT_ENTRIES];
int_ptr_t   int_ptr;

void pic_int_set_gate(uint8_t index, uint64_t offset, uint16_t selector, uint8_t attr){
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

void set_pic_int_descriptor_table(){
    // Software Interrupts
    pic_int_set_gate(0, (uint64_t)&isr0 ,  0x8, 0x8E);    // Division By Zero
    pic_int_set_gate(1, (uint64_t)&isr1 ,  0x8, 0x8E);    // Debug
    pic_int_set_gate(2, (uint64_t)&isr2 ,  0x8, 0x8E);    // Non Maskable Interrupt  
    pic_int_set_gate(3, (uint64_t)&isr3 ,  0x8, 0x8E);    // Breakpoint 
    pic_int_set_gate(4, (uint64_t)&isr4 ,  0x8, 0x8E);    // Into Detected Overflow
    pic_int_set_gate(5, (uint64_t)&isr5 ,  0x8, 0x8E);    // Out of Bounds
    pic_int_set_gate(6, (uint64_t)&isr6 ,  0x8, 0x8E);    // Invalid Opcode
    pic_int_set_gate(7, (uint64_t)&isr7 ,  0x8, 0x8E);    // No Coprocessor
    pic_int_set_gate(8, (uint64_t)&isr8 ,  0x8, 0x8E);    // Double fault (pushes an error code)
    pic_int_set_gate(9, (uint64_t)&isr9 ,  0x8, 0x8E);    // Coprocessor Segment Overrun
    pic_int_set_gate(10, (uint64_t)&isr10 , 0x8, 0x8E);    // Bad TSS (pushes an error code)
    pic_int_set_gate(11, (uint64_t)&isr11 , 0x8, 0x8E);    // Segment not present (pushes an error code)
    pic_int_set_gate(12, (uint64_t)&isr12 , 0x8, 0x8E);    // Stack fault (pushes an error code)
    pic_int_set_gate(13, (uint64_t)&isr13 , 0x8, 0x8E);    // General protection fault (pushes an error code)
    pic_int_set_gate(14, (uint64_t)&isr14 , 0x8, 0x8E);    // Page fault (pushes an error code)
    pic_int_set_gate(15, (uint64_t)&isr15 , 0x8, 0x8E);    // Unknown Interrupt
    pic_int_set_gate(16, (uint64_t)&isr16 , 0x8, 0x8E);    // Coprocessor Fault
    pic_int_set_gate(17, (uint64_t)&isr17 , 0x8, 0x8E);    // Alignment Fault
    pic_int_set_gate(18, (uint64_t)&isr18 , 0x8, 0x8E);    // Machine Check
    pic_int_set_gate(19, (uint64_t)&isr19 , 0x8, 0x8E);    // SIMD (SSE/AVX) error
    pic_int_set_gate(20, (uint64_t)&isr20 , 0x8, 0x8E);    // Reserved
    pic_int_set_gate(21, (uint64_t)&isr21 , 0x8, 0x8E);    // Reserved
    pic_int_set_gate(22, (uint64_t)&isr22 , 0x8, 0x8E);    // Reserved
    pic_int_set_gate(23, (uint64_t)&isr23 , 0x8, 0x8E);    // Reserved
    pic_int_set_gate(24, (uint64_t)&isr24 , 0x8, 0x8E);    // Reserved
    pic_int_set_gate(25, (uint64_t)&isr25 , 0x8, 0x8E);    // Reserved
    pic_int_set_gate(26, (uint64_t)&isr26 , 0x8, 0x8E);    // Reserved
    pic_int_set_gate(27, (uint64_t)&isr27 , 0x8, 0x8E);    // Reserved
    pic_int_set_gate(28, (uint64_t)&isr28 , 0x8, 0x8E);    // Reserved
    pic_int_set_gate(29, (uint64_t)&isr29 , 0x8, 0x8E);    // Reserved
    pic_int_set_gate(30, (uint64_t)&isr30 , 0x8, 0x8E);    // Reserved
    pic_int_set_gate(31, (uint64_t)&isr31 , 0x8, 0x8E);    // Reserved

    // Hardware Interrupts
    pic_int_set_gate(32, (uint64_t)&irq0, 0x08, 0x8E);    // Timer Interrupt, IRQ0
    pic_int_set_gate(33, (uint64_t)&irq1, 0x08, 0x8E);    // Keyboard Interrupt, IRQ1
    // pic_int_set_gate(34, (uint64_t)&irq2, 0x08, 0x8E);    // Cascade (for PIC chaining), IRQ2
    pic_int_set_gate(35, (uint64_t)&irq3, 0x08, 0x8E);    // COM2 (Serial Port 2), IRQ3
    pic_int_set_gate(36, (uint64_t)&irq4, 0x08, 0x8E);    // COM1 (Serial Port 1), IRQ4
    pic_int_set_gate(37, (uint64_t)&irq5, 0x08, 0x8E);    // LPT2 (Parallel Port 2) or Sound Card, IRQ5
    pic_int_set_gate(38, (uint64_t)&irq6, 0x08, 0x8E);    // Floppy Disk Controller, IRQ6
    pic_int_set_gate(39, (uint64_t)&irq7, 0x08, 0x8E);    // LPT1 (Parallel Port 1) / Spurious IRQ, IRQ7
    pic_int_set_gate(40, (uint64_t)&irq8, 0x08, 0x8E);    // Real-Time Clock (RTC), IRQ8
    pic_int_set_gate(41, (uint64_t)&irq9, 0x08, 0x8E);    // ACPI / General system use, IRQ9
    pic_int_set_gate(42, (uint64_t)&irq10, 0x08, 0x8E);   // Available (often used for SCSI or NIC), IRQ10
    pic_int_set_gate(43, (uint64_t)&irq11, 0x08, 0x8E);   // Available (often used for PCI devices), IRQ11
    pic_int_set_gate(44, (uint64_t)&irq12, 0x08, 0x8E);   // PS/2 Mouse, IRQ12
    pic_int_set_gate(45, (uint64_t)&irq13, 0x08, 0x8E);   // FPU / Floating-Point Unit (Coprocessor), IRQ13
    pic_int_set_gate(46, (uint64_t)&irq14, 0x08, 0x8E);   // Primary ATA Hard Disk Controller, IRQ14
    pic_int_set_gate(47, (uint64_t)&irq15, 0x08, 0x8E);   // Secondary ATA Hard Disk Controller, IRQ15
    // pic_int_set_gate(48, (uint64_t)&irq16, 0x08, 0x8E);   // APIC Timer, IRQ16
    // pic_int_set_gate(49, (uint64_t)&irq17, 0x08, 0x8E);   // HPET Timer, IRQ17
    // pic_int_set_gate(50, (uint64_t)&irq18, 0x08, 0x8E);   // Available, IRQ18

    // System Calls
    // pic_int_set_gate(89, (uint64_t)&irq57, 0x08, 0xEE); // Print System Call, IRQ140
    // pic_int_set_gate(90, (uint64_t)&irq58, 0x08, 0xEE); // Read System Call, IRQ141
    // pic_int_set_gate(91, (uint64_t)&irq59, 0x08, 0xEE); // Exit System Call, IRQ142
}


// Initialize the Interrupt Descriptor Table (IDT) for bootstrap cpu core
void pic_int_init(){

    asm volatile("cli");

    // for safety clearing memories
    memset((void *)&int_entries, 0, (sizeof(int_entry_t) *  TOTAL_INT_ENTRIES));
    set_pic_int_descriptor_table();

    memset((void *) &int_ptr, 0, sizeof(int_ptr_t));
    uint16_t limit = (sizeof(int_entry_t) *  TOTAL_INT_ENTRIES) - 1;
    uint64_t base  = (uint64_t) &int_entries;
    int_ptr.limit = limit;
    int_ptr.base = base;

    idt_flush((uint64_t) &int_ptr);

    enable_pic();       // Enable the PIC after setting up the IDT
    pic_irq_remap();    // Remap the PIC to the new interrupt vector table

    asm volatile("sti");

    printf("[Info] Successfully pic Bootstrap Interrupt Initialized.\n");
}


// Testing Interrupts and Debugging
void test_interrupt(uint64_t int_no){
    if(int_no > 255){
        printf("[Error] Invalid Interrupt Number: %d\n", int_no);
        return;
    }
    printf("Testing Interrupts\n");
    printf("Interrupt Number: %d\n", int_no);

    switch(int_no){
        case 172:
            asm volatile("int $172"); // Print System Call
            break;
        case 173:
            asm volatile("int $172"); // Read System Call
            break;
        case 174:
            asm volatile("int $173"); // Exit System Call
            break;
        case 3:
            asm volatile("int $3");
            break;
        case 4:
            asm volatile("int $4");
            break;
        case 5:
            asm volatile("int $5");
            break;
        case 6:
            asm volatile("int $6");
            break;
        case 7:
            asm volatile("int $7");
            break;
        default:
            printf("[Error] Interrupt %d is not implemented.\n", int_no);
    }
}

