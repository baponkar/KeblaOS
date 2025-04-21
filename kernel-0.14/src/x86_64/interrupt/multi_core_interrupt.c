/*
This file will hold general function which will
be use in pic and apic interrupt.

The Interrupt Service Routine (ISR) is the function that is called when an cpu exception interrupt is triggered.
The Interrupt Request (IRQ) is the function that is called when an hardware interrupt is triggered.
*/

#include "../timer/apic_timer.h"
#include "../../driver/io/ports.h" // for outb
#include "../../lib/stdio.h" //for printf
#include "../../memory/paging.h" // for page_fault_handler
#include "../../lib/string.h"
#include "../../cpu/cpu.h"
#include "apic.h"
#include "interrupt.h"
#include "isr_manage.h"
#include "irq_manage.h"

#include "multi_core_interrupt.h"

#define PIC_COMMAND_MASTER 0x20
#define PIC_COMMAND_SLAVE 0xA0
#define PIC_DATA_MASTER 0x21
#define PIC_DATA_SLAVE 0xA1
#define PIC_EOI 0x20 // End of Interrupt

#define MAX_CPU_COUNT 256           // Maximum CPU cores supported

#define GET_IRQ(INT_VECTOR) (INT_VECTOR - 32)   // Get IRQ from Interrupt Vector

extern bool has_apic();
extern void idt_flush(uint64_t);
extern void load_tss(uint64_t);

int_entry_t core_int_entries[MAX_CPU_COUNT][256]; // 256 entries for each core

int_ptr_t core_int_ptr[MAX_CPU_COUNT];  // 256 entries for each core



void core_int_set_gate(uint64_t core_id, uint8_t index, uint64_t offset, uint16_t selector, uint8_t attr){
    int_entry_t *entry = (int_entry_t *) &core_int_entries[core_id][index];

    entry->offset_1 = (uint16_t) (offset & 0xFFFF); // set lower 16 bit
    entry->offset_2 = (uint16_t) (offset >> 16) & 0xFFFF; // set 16 bit
    entry->offset_3 = (uint32_t) (offset >> 32) & 0xFFFFFFFF; // set upper 32 bit

    entry->selector = selector;                   // set 16 bit of selector
    //              |P|DPL|R|TYPE|
    // for x86_64 : |1|00 |0|1110| ==> 10001110 ==> 0x8E
    entry->type_attributes = attr;    // set 8 bit  of P(1 bit) + DPL(2 bit) + gate type(4 bit) + 0(1 bit)
    
    entry->ist = 0; // disabled ist i.e clear 3 bit of ist and 5 bit of reserved field 
    entry->zero = 0; // set top 32 bit to zero
}


void set_core_descriptor_table(uint64_t core_id){

    // Setting Interrupts Service Routine Gate(ISR Gate)
    // https://stackoverflow.com/questions/9113310/segment-selector-in-ia-32
    // selector = 0x08 = 0b1000, 64-bit Interrupt Gate => attr = 0x8E = 1 0 00 1110, (p=0b1,0b0, dpl=0b00, gate type=0b1110)
    // selector value is 1000 because GDT code segment index is 1
    // selector = index + table_to_use + privilege
    // selector  = 1<<3(index 1) + 0<<2(TI for GDT 0) + 0<<1(for ring 0) => 1000 + 000 + 00 = 1000 = 0x08
    // attribute = P | DPL | 0 | Gate Type        | 
    // attribute = 1 | 00  | 0 | 1110 (interrupt) | = 0x8E for DPL = 0
    // attribute = 1 | 11  | 0 | 1110 (interrupt) | = 0xEE for DPL = 3

    // Define Interrupt for DPL = 0
    core_int_set_gate(core_id,  0, (uint64_t)&isr0 ,  0x8, 0x8E);    // Division By Zero
    core_int_set_gate(core_id,  1, (uint64_t)&isr1 ,  0x8, 0x8E);    // Debug
    core_int_set_gate(core_id,  2, (uint64_t)&isr2 ,  0x8, 0x8E);    // Non Maskable Interrupt  
    core_int_set_gate(core_id,  3, (uint64_t)&isr3 ,  0x8, 0x8E);    // Breakpoint 
    core_int_set_gate(core_id,  4, (uint64_t)&isr4 ,  0x8, 0x8E);    // Into Detected Overflow
    core_int_set_gate(core_id,  5, (uint64_t)&isr5 ,  0x8, 0x8E);    // Out of Bounds
    core_int_set_gate(core_id,  6, (uint64_t)&isr6 ,  0x8, 0x8E);    // Invalid Opcode
    core_int_set_gate(core_id,  7, (uint64_t)&isr7 ,  0x8, 0x8E);    // No Coprocessor
    core_int_set_gate(core_id,  8, (uint64_t)&isr8 ,  0x8, 0x8E);    // Double fault (pushes an error code)
    core_int_set_gate(core_id,  9, (uint64_t)&isr9 ,  0x8, 0x8E);    // Coprocessor Segment Overrun
    core_int_set_gate(core_id, 10, (uint64_t)&isr10 , 0x8, 0x8E);    // Bad TSS (pushes an error code)
    core_int_set_gate(core_id, 11, (uint64_t)&isr11 , 0x8, 0x8E);    // Segment not present (pushes an error code)
    core_int_set_gate(core_id, 12, (uint64_t)&isr12 , 0x8, 0x8E);    // Stack fault (pushes an error code)
    core_int_set_gate(core_id, 13, (uint64_t)&isr13 , 0x8, 0x8E);    // General protection fault (pushes an error code)
    core_int_set_gate(core_id, 14, (uint64_t)&isr14 , 0x8, 0x8E);    // Page fault (pushes an error code)
    core_int_set_gate(core_id, 15, (uint64_t)&isr15 , 0x8, 0x8E);    // Unknown Interrupt
    core_int_set_gate(core_id, 16, (uint64_t)&isr16 , 0x8, 0x8E);    // Coprocessor Fault
    core_int_set_gate(core_id, 17, (uint64_t)&isr17 , 0x8, 0x8E);    // Alignment Fault
    core_int_set_gate(core_id, 18, (uint64_t)&isr18 , 0x8, 0x8E);    // Machine Check
    core_int_set_gate(core_id, 19, (uint64_t)&isr19 , 0x8, 0x8E);    // SIMD (SSE/AVX) error
    core_int_set_gate(core_id, 20, (uint64_t)&isr20 , 0x8, 0x8E);    // Reserved
    core_int_set_gate(core_id, 21, (uint64_t)&isr21 , 0x8, 0x8E);    // Reserved
    core_int_set_gate(core_id, 22, (uint64_t)&isr22 , 0x8, 0x8E);    // Reserved
    core_int_set_gate(core_id, 23, (uint64_t)&isr23 , 0x8, 0x8E);    // Reserved
    core_int_set_gate(core_id, 24, (uint64_t)&isr24 , 0x8, 0x8E);    // Reserved
    core_int_set_gate(core_id, 25, (uint64_t)&isr25 , 0x8, 0x8E);    // Reserved
    core_int_set_gate(core_id, 26, (uint64_t)&isr26 , 0x8, 0x8E);    // Reserved
    core_int_set_gate(core_id, 27, (uint64_t)&isr27 , 0x8, 0x8E);    // Reserved
    core_int_set_gate(core_id, 28, (uint64_t)&isr28 , 0x8, 0x8E);    // Reserved
    core_int_set_gate(core_id, 29, (uint64_t)&isr29 , 0x8, 0x8E);    // Reserved
    core_int_set_gate(core_id, 30, (uint64_t)&isr30 , 0x8, 0x8E);    // Reserved
    core_int_set_gate(core_id, 31, (uint64_t)&isr31 , 0x8, 0x8E);    // Reserved

    core_int_set_gate(core_id, 32, (uint64_t)&irq0, 0x08, 0x8E);    // Timer Interrupt, IRQ0
    core_int_set_gate(core_id, 33, (uint64_t)&irq1, 0x08, 0x8E);    // Keyboard Interrupt, IRQ1
    core_int_set_gate(core_id, 34, (uint64_t)&irq2, 0x08, 0x8E);    // Cascade (for PIC chaining), IRQ2
    core_int_set_gate(core_id, 35, (uint64_t)&irq3, 0x08, 0x8E);    // COM2 (Serial Port 2), IRQ3
    core_int_set_gate(core_id, 36, (uint64_t)&irq4, 0x08, 0x8E);    // COM1 (Serial Port 1), IRQ4
    core_int_set_gate(core_id, 37, (uint64_t)&irq5, 0x08, 0x8E);    // LPT2 (Parallel Port 2) or Sound Card, IRQ5
    core_int_set_gate(core_id, 38, (uint64_t)&irq6, 0x08, 0x8E);    // Floppy Disk Controller, IRQ6
    core_int_set_gate(core_id, 39, (uint64_t)&irq7, 0x08, 0x8E);    // LPT1 (Parallel Port 1) / Spurious IRQ, IRQ7
    core_int_set_gate(core_id, 40, (uint64_t)&irq8, 0x08, 0x8E);    // Real-Time Clock (RTC), IRQ8
    core_int_set_gate(core_id, 41, (uint64_t)&irq9, 0x08, 0x8E);    // ACPI / General system use, IRQ9

    core_int_set_gate(core_id, 42, (uint64_t)&irq10, 0x08, 0x8E);   // Available (often used for SCSI or NIC), IRQ10
    core_int_set_gate(core_id, 43, (uint64_t)&irq11, 0x08, 0x8E);   // Available (often used for PCI devices), IRQ11

    core_int_set_gate(core_id, 44, (uint64_t)&irq12, 0x08, 0x8E);   // PS/2 Mouse, IRQ12
    
    core_int_set_gate(core_id, 45, (uint64_t)&irq13, 0x08, 0x8E);   // FPU / Floating-Point Unit (Coprocessor), IRQ13

    core_int_set_gate(core_id, 46, (uint64_t)&irq14, 0x08, 0x8E);   // Primary ATA Hard Disk Controller, IRQ14
    core_int_set_gate(core_id, 47, (uint64_t)&irq15, 0x08, 0x8E);   // Secondary ATA Hard Disk Controller, IRQ15

    core_int_set_gate(core_id, 48, (uint64_t)&irq16, 0x08, 0x8E);   // APIC Timer, IRQ16
    core_int_set_gate(core_id, 49, (uint64_t)&irq17, 0x08, 0x8E);   // HPET Timer, IRQ17
    core_int_set_gate(core_id, 50, (uint64_t)&irq18, 0x08, 0x8E);   // Available, IRQ18

    // Define Interrupt for DPL = 3
    core_int_set_gate(core_id, 172, (uint64_t)&irq140, 0x08, 0xEE); // Print System Call, IRQ140
    core_int_set_gate(core_id, 173, (uint64_t)&irq141, 0x08, 0xEE); // Read System Call, IRQ141
    core_int_set_gate(core_id, 174, (uint64_t)&irq142, 0x08, 0xEE); // Exit System Call, IRQ142
}


void init_core_interrupt(uint64_t core_id){
    asm volatile("cli");

    // Setting up the Interrupt Descriptor Table Register
    core_int_ptr[core_id].limit = (sizeof(int_entry_t) * 256) - 1;
    core_int_ptr[core_id].base  = (uint64_t) &core_int_entries[core_id];

    // for safety clearing memories
    memset((void *)&core_int_entries[core_id], 0, (size_t) (sizeof(int_entry_t) * 256));

    // Setting up the Interrupt Descriptor Table
    set_core_descriptor_table(core_id);

    // Load the core's IDT
    idt_flush((uint64_t) &core_int_ptr[core_id]);
   
    asm volatile("sti");
    printf(" [-] Successfully CPU %d Interrupt Initialized.\n", core_id);
}


void init_bootstrap_interrupt(int bootstrap_core_id){
    init_core_interrupt(bootstrap_core_id);
    
    printf("[Info] Successfully Bootstrap CPU %d Interrupt Initialized.\n", bootstrap_core_id);
}


void init_application_core_interrupt(int start_core_id, int end_core_id){
    for (int core_id = start_core_id; core_id <= end_core_id; core_id++) {
        init_core_interrupt(core_id);
        printf("[Info] Successfully Application CPU %d Interrupt Initialized.\n", core_id);
    }
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




