/*

*/

#include "../../../../../limine-9.2.3/limine.h"     // for smp_response

#include "../isr_manage.h"
#include "../irq_manage.h"

#include "../../../lib/stdio.h"
#include "../../../lib/string.h"

#include "ioapic.h"
#include "apic.h"

#include "../../../kshell/kshell.h"

#include "apic_interrupt.h"


#define  TOTAL_INT_ENTRIES 256

extern struct limine_smp_response *smp_response;

extern void idt_flush(uint64_t);

extern int_entry_t int_entries[TOTAL_INT_ENTRIES];
extern int_ptr_t   int_ptr;



void apic_int_set_gate(uint8_t index, uint64_t offset, uint16_t selector, uint8_t attr){
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

// Set the IDT pointer and size for BOOTSTRAP_CORE
void set_bsp_apic_int_descriptor_table(){
    // Software Interrupts
    apic_int_set_gate(0, (uint64_t)&isr0 ,  0x8, 0x8E);    // Division By Zero
    apic_int_set_gate(1, (uint64_t)&isr1 ,  0x8, 0x8E);    // Debug
    apic_int_set_gate(2, (uint64_t)&isr2 ,  0x8, 0x8E);    // Non Maskable Interrupt  
    apic_int_set_gate(3, (uint64_t)&isr3 ,  0x8, 0x8E);    // Breakpoint 
    apic_int_set_gate(4, (uint64_t)&isr4 ,  0x8, 0x8E);    // Into Detected Overflow
    apic_int_set_gate(5, (uint64_t)&isr5 ,  0x8, 0x8E);    // Out of Bounds
    apic_int_set_gate(6, (uint64_t)&isr6 ,  0x8, 0x8E);    // Invalid Opcode
    apic_int_set_gate(7, (uint64_t)&isr7 ,  0x8, 0x8E);    // No Coprocessor
    apic_int_set_gate(8, (uint64_t)&isr8 ,  0x8, 0x8E);    // Double fault (pushes an error code)
    apic_int_set_gate(9, (uint64_t)&isr9 ,  0x8, 0x8E);    // Coprocessor Segment Overrun
    apic_int_set_gate(10, (uint64_t)&isr10 , 0x8, 0x8E);    // Bad TSS (pushes an error code)
    apic_int_set_gate(11, (uint64_t)&isr11 , 0x8, 0x8E);    // Segment not present (pushes an error code)
    apic_int_set_gate(12, (uint64_t)&isr12 , 0x8, 0x8E);    // Stack fault (pushes an error code)
    apic_int_set_gate(13, (uint64_t)&isr13 , 0x8, 0x8E);    // General protection fault (pushes an error code)
    apic_int_set_gate(14, (uint64_t)&isr14 , 0x8, 0x8E);    // Page fault (pushes an error code)
    apic_int_set_gate(15, (uint64_t)&isr15 , 0x8, 0x8E);    // Unknown Interrupt
    apic_int_set_gate(16, (uint64_t)&isr16 , 0x8, 0x8E);    // Coprocessor Fault
    apic_int_set_gate(17, (uint64_t)&isr17 , 0x8, 0x8E);    // Alignment Fault
    apic_int_set_gate(18, (uint64_t)&isr18 , 0x8, 0x8E);    // Machine Check
    apic_int_set_gate(19, (uint64_t)&isr19 , 0x8, 0x8E);    // SIMD (SSE/AVX) error
    apic_int_set_gate(20, (uint64_t)&isr20 , 0x8, 0x8E);    // Reserved
    apic_int_set_gate(21, (uint64_t)&isr21 , 0x8, 0x8E);    // Reserved
    apic_int_set_gate(22, (uint64_t)&isr22 , 0x8, 0x8E);    // Reserved
    apic_int_set_gate(23, (uint64_t)&isr23 , 0x8, 0x8E);    // Reserved
    apic_int_set_gate(24, (uint64_t)&isr24 , 0x8, 0x8E);    // Reserved
    apic_int_set_gate(25, (uint64_t)&isr25 , 0x8, 0x8E);    // Reserved
    apic_int_set_gate(26, (uint64_t)&isr26 , 0x8, 0x8E);    // Reserved
    apic_int_set_gate(27, (uint64_t)&isr27 , 0x8, 0x8E);    // Reserved
    apic_int_set_gate(28, (uint64_t)&isr28 , 0x8, 0x8E);    // Reserved
    apic_int_set_gate(29, (uint64_t)&isr29 , 0x8, 0x8E);    // Reserved
    apic_int_set_gate(30, (uint64_t)&isr30 , 0x8, 0x8E);    // Reserved
    apic_int_set_gate(31, (uint64_t)&isr31 , 0x8, 0x8E);    // Reserved

    // Hardware Interrupts
    apic_int_set_gate(32, (uint64_t)&irq0, 0x08, 0x8E);    // Timer Interrupt, IRQ0
    apic_int_set_gate(33, (uint64_t)&irq1, 0x08, 0x8E);    // Keyboard Interrupt, IRQ1
    apic_int_set_gate(34, (uint64_t)&irq2, 0x08, 0x8E);    // Cascade (for PIC chaining), IRQ2
    apic_int_set_gate(35, (uint64_t)&irq3, 0x08, 0x8E);    // COM2 (Serial Port 2), IRQ3
    apic_int_set_gate(36, (uint64_t)&irq4, 0x08, 0x8E);    // COM1 (Serial Port 1), IRQ4
    apic_int_set_gate(37, (uint64_t)&irq5, 0x08, 0x8E);    // LPT2 (Parallel Port 2) or Sound Card, IRQ5
    apic_int_set_gate(38, (uint64_t)&irq6, 0x08, 0x8E);    // Floppy Disk Controller, IRQ6
    apic_int_set_gate(39, (uint64_t)&irq7, 0x08, 0x8E);    // LPT1 (Parallel Port 1) / Spurious IRQ, IRQ7
    apic_int_set_gate(40, (uint64_t)&irq8, 0x08, 0x8E);    // Real-Time Clock (RTC), IRQ8
    apic_int_set_gate(41, (uint64_t)&irq9, 0x08, 0x8E);    // ACPI / General system use, IRQ9
    apic_int_set_gate(42, (uint64_t)&irq10, 0x08, 0x8E);   // Available (often used for SCSI or NIC), IRQ10
    apic_int_set_gate(43, (uint64_t)&irq11, 0x08, 0x8E);   // Available (often used for PCI devices), IRQ11
    apic_int_set_gate(44, (uint64_t)&irq12, 0x08, 0x8E);   // PS/2 Mouse, IRQ12
    apic_int_set_gate(45, (uint64_t)&irq13, 0x08, 0x8E);   // FPU / Floating-Point Unit (Coprocessor), IRQ13
    apic_int_set_gate(46, (uint64_t)&irq14, 0x08, 0x8E);   // Primary ATA Hard Disk Controller, IRQ14
    apic_int_set_gate(47, (uint64_t)&irq15, 0x08, 0x8E);   // Secondary ATA Hard Disk Controller, IRQ15
    apic_int_set_gate(48, (uint64_t)&irq16, 0x08, 0x8E);   // APIC Timer, IRQ16
    apic_int_set_gate(49, (uint64_t)&irq17, 0x08, 0x8E);   // HPET Timer, IRQ17
    
    apic_int_set_gate(50, (uint64_t)&irq18, 0x08, 0x8E);   // IPI, IRQ18
    apic_int_set_gate(51, (uint64_t)&irq19, 0x08, 0xEE);   // IPI, IRQ19

    // System Calls
    apic_int_set_gate(128, (uint64_t)&irq96, 0x08, 0xEE);  // System Call
}


void bsp_apic_irq_remap(){
    uint8_t bsp_id = smp_response->bsp_lapic_id;            // Bootstrap CPU ID
    uint32_t bsp_flags = (0 << 8) | (0 << 13) | (0 << 15);  // Flags for the BSP

    ioapic_route_all_irq(bsp_id, bsp_flags);
}


// Initialize the Interrupt Descriptor Table (IDT) for bootstrap cpu core
void bsp_apic_int_init(){

    asm volatile("cli");

    // for safety clearing memories
    memset((void *)&int_entries, 0, (sizeof(int_entry_t) *  TOTAL_INT_ENTRIES));
    set_bsp_apic_int_descriptor_table();

    memset((void *) &int_ptr, 0, sizeof(int_ptr_t));
    uint16_t limit = (sizeof(int_entry_t) *  TOTAL_INT_ENTRIES) - 1;
    uint64_t base  = (uint64_t) &int_entries;
    int_ptr.limit = limit;
    int_ptr.base = base;

    idt_flush((uint64_t) &int_ptr);

    enable_apic();          // Enable the APIC after setting up the IDT
    enable_ioapic_mode();   // Enable IOAPIC mode
    bsp_apic_irq_remap();   // Remap the APIC to the new interrupt vector table

    asm volatile("sti");

    printf(" [-] Successfully Bootstrap apic Interrupt Initialized.\n");
}


// Initialize the Interrupt Descriptor Table (IDT) for Application CPU cores

#define MAX_CPU_COUNT 256                           // Maximum CPU cores supported

#define GET_IRQ(INT_VECTOR) (INT_VECTOR - 32)       // Get IRQ from Interrupt Vector

int_entry_t core_int_entries[MAX_CPU_COUNT][256];   // 256 entries for each core

int_ptr_t core_int_ptr[MAX_CPU_COUNT];              // 256 entries for each core

void ap_int_set_gate(uint64_t core_id, uint8_t index, uint64_t offset, uint16_t selector, uint8_t attr){
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



void set_ap_descriptor_table(uint64_t core_id){

    // Setting Interrupts Service Routine Gate(ISR Gate)
    // https://stackoverflow.com/questions/9113310/segment-selector-in-ia-32
    // selector = 0x08 = 0b1000, 64-bit Interrupt Gate => attr = 0x8E = 1 0 00 1110, (p=0b1,0b0, dpl=0b00, gate type=0b1110)
    // selector value is 1000 because GDT code segment index is 1
    // selector = index + table_to_use + privilege
    // selector  = 1<<3(index 1) + 0<<2(TI for GDT 0) + 0<<1(for ring 0) => 1000 + 000 + 00 = 1000 = 0x08
    // attribute = P | DPL | 0 | Gate Type        | 
    // attribute = 1 | 00  | 0 | 1110 (interrupt) | = 0x8E for DPL = 0
    // attribute = 1 | 11  | 0 | 1110 (interrupt) | = 0xEE for DPL = 3

    // Hardware Interrupts
    ap_int_set_gate(core_id,  0, (uint64_t)&isr0 ,  0x8, 0x8E);    // Division By Zero
    ap_int_set_gate(core_id,  1, (uint64_t)&isr1 ,  0x8, 0x8E);    // Debug
    ap_int_set_gate(core_id,  2, (uint64_t)&isr2 ,  0x8, 0x8E);    // Non Maskable Interrupt  
    ap_int_set_gate(core_id,  3, (uint64_t)&isr3 ,  0x8, 0x8E);    // Breakpoint 
    ap_int_set_gate(core_id,  4, (uint64_t)&isr4 ,  0x8, 0x8E);    // Into Detected Overflow
    ap_int_set_gate(core_id,  5, (uint64_t)&isr5 ,  0x8, 0x8E);    // Out of Bounds
    ap_int_set_gate(core_id,  6, (uint64_t)&isr6 ,  0x8, 0x8E);    // Invalid Opcode
    ap_int_set_gate(core_id,  7, (uint64_t)&isr7 ,  0x8, 0x8E);    // No Coprocessor
    ap_int_set_gate(core_id,  8, (uint64_t)&isr8 ,  0x8, 0x8E);    // Double fault (pushes an error code)
    ap_int_set_gate(core_id,  9, (uint64_t)&isr9 ,  0x8, 0x8E);    // Coprocessor Segment Overrun
    ap_int_set_gate(core_id, 10, (uint64_t)&isr10 , 0x8, 0x8E);    // Bad TSS (pushes an error code)
    ap_int_set_gate(core_id, 11, (uint64_t)&isr11 , 0x8, 0x8E);    // Segment not present (pushes an error code)
    ap_int_set_gate(core_id, 12, (uint64_t)&isr12 , 0x8, 0x8E);    // Stack fault (pushes an error code)
    ap_int_set_gate(core_id, 13, (uint64_t)&isr13 , 0x8, 0x8E);    // General protection fault (pushes an error code)
    ap_int_set_gate(core_id, 14, (uint64_t)&isr14 , 0x8, 0x8E);    // Page fault (pushes an error code)
    ap_int_set_gate(core_id, 15, (uint64_t)&isr15 , 0x8, 0x8E);    // Unknown Interrupt
    ap_int_set_gate(core_id, 16, (uint64_t)&isr16 , 0x8, 0x8E);    // Coprocessor Fault
    ap_int_set_gate(core_id, 17, (uint64_t)&isr17 , 0x8, 0x8E);    // Alignment Fault
    ap_int_set_gate(core_id, 18, (uint64_t)&isr18 , 0x8, 0x8E);    // Machine Check
    ap_int_set_gate(core_id, 19, (uint64_t)&isr19 , 0x8, 0x8E);    // SIMD (SSE/AVX) error
    ap_int_set_gate(core_id, 20, (uint64_t)&isr20 , 0x8, 0x8E);    // Reserved
    ap_int_set_gate(core_id, 21, (uint64_t)&isr21 , 0x8, 0x8E);    // Reserved
    ap_int_set_gate(core_id, 22, (uint64_t)&isr22 , 0x8, 0x8E);    // Reserved
    ap_int_set_gate(core_id, 23, (uint64_t)&isr23 , 0x8, 0x8E);    // Reserved
    ap_int_set_gate(core_id, 24, (uint64_t)&isr24 , 0x8, 0x8E);    // Reserved
    ap_int_set_gate(core_id, 25, (uint64_t)&isr25 , 0x8, 0x8E);    // Reserved
    ap_int_set_gate(core_id, 26, (uint64_t)&isr26 , 0x8, 0x8E);    // Reserved
    ap_int_set_gate(core_id, 27, (uint64_t)&isr27 , 0x8, 0x8E);    // Reserved
    ap_int_set_gate(core_id, 28, (uint64_t)&isr28 , 0x8, 0x8E);    // Reserved
    ap_int_set_gate(core_id, 29, (uint64_t)&isr29 , 0x8, 0x8E);    // Reserved
    ap_int_set_gate(core_id, 30, (uint64_t)&isr30 , 0x8, 0x8E);    // Reserved
    ap_int_set_gate(core_id, 31, (uint64_t)&isr31 , 0x8, 0x8E);    // Reserved

    ap_int_set_gate(core_id, 48, (uint64_t)&irq16, 0x08, 0x8E);   // APIC Timer, IRQ16

    // Software Interrupts
    // Bootstrap Core has already set up the IOAPIC for hardware interrupts
    ap_int_set_gate(core_id, 50, (uint64_t)&irq18, 0x08, 0xEE); // IPI, IRQ18
    ap_int_set_gate(core_id, 51, (uint64_t)&irq19, 0x08, 0xEE); // IPI, IRQ18
    
    // System Calls
    ap_int_set_gate(core_id, 128, (uint64_t)&irq96, 0x08, 0xEE);    //  System Call
}


void ap_apic_int_init(uint64_t core_id){
    
    asm volatile("cli");

    // Setting up the Interrupt Descriptor Table Register
    core_int_ptr[core_id].limit = (sizeof(int_entry_t) * 256) - 1;
    core_int_ptr[core_id].base  = (uint64_t) &core_int_entries[core_id];

    // for safety clearing memories
    memset((void *)&core_int_entries[core_id], 0, (size_t) (sizeof(int_entry_t) * 256));

    // Setting up the Interrupt Descriptor Table
    set_ap_descriptor_table(core_id);

    // Load the core's IDT
    idt_flush((uint64_t) &core_int_ptr[core_id]);
   
    asm volatile("sti");
    printf(" [-] Successfully CPU %d Interrupt Initialized.\n", core_id);
}

