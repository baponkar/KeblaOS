/*
This file will hold general function which will
be use in pic and apic interrupt.
*/
#include "../timer/apic_timer.h"
#include "../../driver/io/ports.h" // for outb
#include "../../lib/stdio.h" //for printf
#include "../../lib/string.h"

#include "interrupt.h"

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

char* exception_messages[] = {
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

int_entry_t boot_int_entries[256];
int_ptr_t boot_int_ptr;

void (*interrupt_routines[224])(registers_t *) = { 0 };

void interrupt_install_handler(int int_no, void (*handler)(registers_t *r))
{
    interrupt_routines[int_no] = handler;
}

void interrupt_uninstall_handler(int int_no)
{
    interrupt_routines[int_no] = 0;
}



void boot_int_set_gate(uint8_t index, uint64_t offset, uint16_t selector, uint8_t attr){
    boot_int_entries[index].offset_1 = (uint16_t) offset & 0xFFFF; // set lower 16 bit
    boot_int_entries[index].offset_2 = (uint16_t) (offset >> 16) & 0xFFFF; // set 16 bit
    boot_int_entries[index].offset_3 = (uint32_t) (offset >> 32) & 0xFFFFFFFF; // set upper 32 bit

    boot_int_entries[index].selector = selector;                   // set 16 bit of selector
    //              |P|DPL|R|TYPE|
    // for x86_64 : |1|00 |0|1110| ==> 10001110 ==> 0x8E
    boot_int_entries[index].type_attributes = attr;    // set 8 bit  of P(1 bit) + DPL(2 bit) + gate type(4 bit) + 0(1 bit)
    
    boot_int_entries[index].ist = 0; // disabled ist i.e clear 3 bit of ist and 5 bit of reserved field 
    boot_int_entries[index].zero = 0; // set top 32 bit to zero
}

void set_boot_descriptor_table(){

    // Setting Interrupts Service Routine Gate(ISR Gate)
    // https://stackoverflow.com/questions/9113310/segment-selector-in-ia-32
    // selector = 0x08 = 0b1000, 64-bit Interrupt Gate => attr = 0x8E = 1 0 00 1110, (p=0b1,0b0, dpl=0b00, gate type=0b1110)
    // selector value is 1000 because GDT code segment index is 1
    // selector = index + table_to_use + privilege
    // selector  = 1<<3(index 1) + 0<<2(TI for GDT 0) + 0<<1(for ring 0) => 1000 + 000 + 00 = 1000 = 0x08

    boot_int_set_gate(  0, (uint64_t)&isr0 ,  0x8, 0x8E);    // Division By Zero
    boot_int_set_gate(  1, (uint64_t)&isr1 ,  0x8, 0x8E);    // Debug
    boot_int_set_gate(  2, (uint64_t)&isr2 ,  0x8, 0x8E);    // Non Maskable Interrupt  
    boot_int_set_gate(  3, (uint64_t)&isr3 ,  0x8, 0x8E);    // Breakpoint 
    boot_int_set_gate(  4, (uint64_t)&isr4 ,  0x8, 0x8E);    // Into Detected Overflow
    boot_int_set_gate(  5, (uint64_t)&isr5 ,  0x8, 0x8E);    // Out of Bounds
    boot_int_set_gate(  6, (uint64_t)&isr6 ,  0x8, 0x8E);    // Invalid Opcode
    boot_int_set_gate(  7, (uint64_t)&isr7 ,  0x8, 0x8E);    // No Coprocessor
    boot_int_set_gate(  8, (uint64_t)&isr8 ,  0x8, 0x8E);    // Double fault (pushes an error code)
    boot_int_set_gate(  9, (uint64_t)&isr9 ,  0x8, 0x8E);    // Coprocessor Segment Overrun
    boot_int_set_gate( 10, (uint64_t)&isr10 , 0x8, 0x8E);    // Bad TSS (pushes an error code)
    boot_int_set_gate( 11, (uint64_t)&isr11 , 0x8, 0x8E);    // Segment not present (pushes an error code)
    boot_int_set_gate( 12, (uint64_t)&isr12 , 0x8, 0x8E);    // Stack fault (pushes an error code)
    boot_int_set_gate( 13, (uint64_t)&isr13 , 0x8, 0x8E);    // General protection fault (pushes an error code)
    boot_int_set_gate( 14, (uint64_t)&isr14 , 0x8, 0x8E);    // Page fault (pushes an error code)
    boot_int_set_gate( 15, (uint64_t)&isr15 , 0x8, 0x8E);    // Unknown Interrupt
    boot_int_set_gate( 16, (uint64_t)&isr16 , 0x8, 0x8E);    // Coprocessor Fault
    boot_int_set_gate( 17, (uint64_t)&isr17 , 0x8, 0x8E);    // Alignment Fault
    boot_int_set_gate( 18, (uint64_t)&isr18 , 0x8, 0x8E);    // Machine Check
    boot_int_set_gate( 19, (uint64_t)&isr19 , 0x8, 0x8E);    // SIMD (SSE/AVX) error
    boot_int_set_gate( 20, (uint64_t)&isr20 , 0x8, 0x8E);    // Reserved
    boot_int_set_gate( 21, (uint64_t)&isr21 , 0x8, 0x8E);    // Reserved
    boot_int_set_gate( 22, (uint64_t)&isr22 , 0x8, 0x8E);    // Reserved
    boot_int_set_gate( 23, (uint64_t)&isr23 , 0x8, 0x8E);    // Reserved
    boot_int_set_gate( 24, (uint64_t)&isr24 , 0x8, 0x8E);    // Reserved
    boot_int_set_gate( 25, (uint64_t)&isr25 , 0x8, 0x8E);    // Reserved
    boot_int_set_gate( 26, (uint64_t)&isr26 , 0x8, 0x8E);    // Reserved
    boot_int_set_gate( 27, (uint64_t)&isr27 , 0x8, 0x8E);    // Reserved
    boot_int_set_gate( 28, (uint64_t)&isr28 , 0x8, 0x8E);    // Reserved
    boot_int_set_gate( 29, (uint64_t)&isr29 , 0x8, 0x8E);    // Reserved
    boot_int_set_gate( 30, (uint64_t)&isr30 , 0x8, 0x8E);    // Reserved
    boot_int_set_gate( 31, (uint64_t)&isr31 , 0x8, 0x8E);    // Reserved

    boot_int_set_gate(32, (uint64_t)&irq0, 0x08, 0x8E);  // Timer Interrupt
    boot_int_set_gate(33, (uint64_t)&irq1, 0x08, 0x8E);  // Keyboard Interrupt
    boot_int_set_gate(34, (uint64_t)&irq2, 0x08, 0x8E);  // Cascade (for PIC chaining)
    boot_int_set_gate(35, (uint64_t)&irq3, 0x08, 0x8E);  // COM2 (Serial Port 2)
    boot_int_set_gate(36, (uint64_t)&irq4, 0x08, 0x8E);  // COM1 (Serial Port 1)
    boot_int_set_gate(37, (uint64_t)&irq5, 0x08, 0x8E);  // LPT2 (Parallel Port 2) or Sound Card
    boot_int_set_gate(38, (uint64_t)&irq6, 0x08, 0x8E);  // Floppy Disk Controller
    boot_int_set_gate(39, (uint64_t)&irq7, 0x08, 0x8E);  // LPT1 (Parallel Port 1) / Spurious IRQ
    boot_int_set_gate(40, (uint64_t)&irq8, 0x08, 0x8E);  // Real-Time Clock (RTC)
    boot_int_set_gate(41, (uint64_t)&irq9, 0x08, 0x8E);  // ACPI / General system use
    boot_int_set_gate(42, (uint64_t)&irq10, 0x08, 0x8E); // Available (often used for SCSI or NIC)
    boot_int_set_gate(43, (uint64_t)&irq11, 0x08, 0x8E); // Available (often used for PCI devices)
    boot_int_set_gate(44, (uint64_t)&irq12, 0x08, 0x8E); // PS/2 Mouse
    boot_int_set_gate(45, (uint64_t)&irq13, 0x08, 0x8E); // FPU / Floating-Point Unit (Coprocessor)
    boot_int_set_gate(46, (uint64_t)&irq14, 0x08, 0x8E); // Primary ATA Hard Disk Controller
    boot_int_set_gate(47, (uint64_t)&irq15, 0x08, 0x8E); // Secondary ATA Hard Disk Controller

    boot_int_set_gate(48, (uint64_t)&irq16, 0x08, 0x8E); // APIC Timer

    boot_int_set_gate(128, (uint64_t)&isr128, 0x8, 0x8E); //System call Write
    boot_int_set_gate(177, (uint64_t)&isr177, 0x8, 0x8E); //System call Read
}


void init_bootstrap_cpu_interrupt(){
    asm volatile("cli");
    boot_int_ptr.limit = (sizeof(int_entry_t) * 256) - 1;
    boot_int_ptr.base  = (uint64_t) &boot_int_entries;
    // for safety clearing memories
    memset((void *)&boot_int_entries, 0, (size_t) (sizeof(int_entry_t) * 256));
    idt_flush((uint64_t) &boot_int_ptr);
    set_boot_descriptor_table();
   
    asm volatile("sti");
    printf("Successfully Bootstrap Interrupt Initialized.\n");
}



#define MAX_CPU_COUNT 256           // Maximum CPU cores supported
int_entry_t core_int_entries[MAX_CPU_COUNT][256];
int_ptr_t core_int_ptr[MAX_CPU_COUNT];


void core_int_set_gate(uint64_t core_id, uint8_t index, uint64_t offset, uint16_t selector, uint8_t attr){
    int_entry_t *entry = &core_int_entries[core_id][index];

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

    core_int_set_gate(core_id, 32, (uint64_t)&irq0, 0x08, 0x8E);  // Timer Interrupt
    core_int_set_gate(core_id, 33, (uint64_t)&irq1, 0x08, 0x8E);  // Keyboard Interrupt
    core_int_set_gate(core_id, 34, (uint64_t)&irq2, 0x08, 0x8E);  // Cascade (for PIC chaining)
    core_int_set_gate(core_id, 35, (uint64_t)&irq3, 0x08, 0x8E);  // COM2 (Serial Port 2)
    core_int_set_gate(core_id, 36, (uint64_t)&irq4, 0x08, 0x8E);  // COM1 (Serial Port 1)
    core_int_set_gate(core_id, 37, (uint64_t)&irq5, 0x08, 0x8E);  // LPT2 (Parallel Port 2) or Sound Card
    core_int_set_gate(core_id, 38, (uint64_t)&irq6, 0x08, 0x8E);  // Floppy Disk Controller
    core_int_set_gate(core_id, 39, (uint64_t)&irq7, 0x08, 0x8E);  // LPT1 (Parallel Port 1) / Spurious IRQ
    core_int_set_gate(core_id, 40, (uint64_t)&irq8, 0x08, 0x8E);  // Real-Time Clock (RTC)
    core_int_set_gate(core_id, 41, (uint64_t)&irq9, 0x08, 0x8E);  // ACPI / General system use
    core_int_set_gate(core_id, 42, (uint64_t)&irq10, 0x08, 0x8E); // Available (often used for SCSI or NIC)
    core_int_set_gate(core_id, 43, (uint64_t)&irq11, 0x08, 0x8E); // Available (often used for PCI devices)
    core_int_set_gate(core_id, 44, (uint64_t)&irq12, 0x08, 0x8E); // PS/2 Mouse
    core_int_set_gate(core_id, 45, (uint64_t)&irq13, 0x08, 0x8E); // FPU / Floating-Point Unit (Coprocessor)
    core_int_set_gate(core_id, 46, (uint64_t)&irq14, 0x08, 0x8E); // Primary ATA Hard Disk Controller
    core_int_set_gate(core_id, 47, (uint64_t)&irq15, 0x08, 0x8E); // Secondary ATA Hard Disk Controller

    core_int_set_gate(core_id, 48, (uint64_t)&irq16, 0x08, 0x8E); // APIC Timer

    core_int_set_gate(core_id, 128, (uint64_t)&isr128, 0x8, 0x8E); //System call Write
    core_int_set_gate(core_id, 177, (uint64_t)&isr177, 0x8, 0x8E); //System call Read
}


void init_core_cpu_interrupt(uint64_t core_id){
    asm volatile("cli");
    core_int_ptr[core_id].limit = (sizeof(int_entry_t) * 256) - 1;
    core_int_ptr[core_id].base  = (uint64_t) &core_int_entries[core_id];
    // for safety clearing memories
    memset((void *)&core_int_entries[core_id], 0, (size_t) (sizeof(int_entry_t) * 256));
    idt_flush((uint64_t) &core_int_ptr[core_id]);
    set_core_descriptor_table(core_id);
   
    asm volatile("sti");
    printf("Successfully CPU %d Interrupt Initialized.\n", core_id);
}






void test_interrupt() {
    // printf("Testing Interrupts\n");
    // asm volatile ("div %b0" :: "a"(0)); // Int no 0
    // asm volatile ("int $0x3");   // Breakpoint int no : 3
    // asm volatile ("int $0x0");   // Division By Zero, int no : 0
    // asm volatile ("int $0xE");   // Page Fault Request, int no: 14
    // asm volatile("int $0xF");    // int no 15
    // asm volatile("int $0x10");   // int no 16
    // asm volatile("int $0x11");   // int no 17
    // asm volatile ("int $0x20");  // Interrupt Request, int no: 32, Timer Interrupt 
    // asm volatile ("int $0x21");  // Interrupt Request, int no : 33, Keyboard Interrupt
    // asm volatile ("int $0x22");  // Interrupt Request, int no: 34
    asm volatile ("int $0x30");     // Interrupt Request, int no: 48
}


void gpf_handler(registers_t *regs){
    printf("recieved interrupt: %d\n", regs->int_no);
    printf("%s\n", exception_messages[regs->int_no]);
    printf("Error Code: %x\n", regs->err_code);
    printf("CS: %x, RIP : %x\n", regs->iret_cs, regs->iret_rip);

    uint64_t *rsp = (uint64_t *)regs->iret_rsp;
    printf("Stack Contents RSP : %x\n", (uint64_t)rsp);
    
    for (int i = 0; i < 26; i++) {
        printf("  [%x] = %x\n", (uint64_t)(rsp + i), rsp[i] );
    }

    // debug_error_code(regs->err_code);
    printf("System Halted!\n");
    halt_kernel();   
}



void debug_error_code(int err_code) {
    // Print the raw error code in decimal and hexadecimal
    printf("Error Code (Decimal): %d\n", err_code);
    printf("Error Code (Hexadecimal): %d\n", err_code);
    // Decode the error code bits
    printf("Decoding Error Code:\n");

    // Bit 0: External Event
    if (err_code & 0x1) {
        printf("  - External: The fault was caused by an external event (e.g., hardware interrupt).\n");
    } else {
        printf("  - External: The fault was not caused by an external event.\n");
    }

    // Bit 1: Table Indicator (GDT/LDT)
    if (err_code & 0x2) {
        printf("  - Table: The fault involves the Local Descriptor Table (LDT).\n");
    } else {
        printf("  - Table: The fault involves the Global Descriptor Table (GDT).\n");
    }

    // Bit 2: Index (IDT/GDT)
    if (err_code & 0x4) {
        printf("  - Index: The fault involves an Interrupt Descriptor Table (IDT) entry.\n");
    } else {
        printf("  - Index: The fault involves a segment selector in the GDT/LDT.\n");
    }

    // Bits 3-15: Segment Selector Index
    int selector_index = (err_code >> 3) & 0x1FFF; // Extract bits 3-15
    printf("  - Selector Index: %d\n", selector_index);

    // Additional information based on the selector index
    if (selector_index == 0) {
        printf("    - Null Selector: The fault was caused by a null segment selector.\n");
    } else {
        printf("    - The fault was caused by a non-null segment selector.\n");
    }

    // Print a summary of the error
    printf("\nSummary:\n");
    if (err_code & 0x4) {
        printf("The fault likely occurred due to an invalid or misconfigured IDT entry.\n");
    } else {
        printf("The fault likely occurred due to an invalid or misconfigured GDT/LDT entry.\n");
    }

    printf("Check the segment selector index (%d)\n", selector_index);
}


