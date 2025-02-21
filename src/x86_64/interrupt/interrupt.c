/*
This file will hold general function which will
be use in pic and apic interrupt.
*/

#include "../../driver/io/ports.h" // for outb
#include "../../lib/stdio.h" //for printf

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

int_entry_t int_entries[256];
int_ptr_t int_ptr;

void *interrupt_routines[16] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

void interrupt_install_handler(int int_no, void (*handler)(registers_t *r))
{
    interrupt_routines[int_no] = handler;
}

void interrupt_uninstall_handler(int int_no)
{
    interrupt_routines[int_no] = 0;
}

void int_set_gate(uint8_t index, uint64_t offset, uint16_t selector, uint8_t attr){
    int_entries[index].offset_1 = (uint16_t) offset & 0xFFFF; // set lower 16 bit
    int_entries[index].offset_2 = (uint16_t) (offset >> 16) & 0xFFFF; // set 16 bit
    int_entries[index].offset_3 = (uint32_t) (offset >> 32) & 0xFFFFFFFF; // set upper 32 bit

    int_entries[index].selector = selector;                   // set 16 bit of selector
    //              |P|DPL|R|TYPE|
    // for x86_64 : |1|00 |0|1110| ==> 10001110 ==> 0x8E
    int_entries[index].type_attributes = attr;    // set 8 bit  of P(1 bit) + DPL(2 bit) + gate type(4 bit) + 0(1 bit)
    
    int_entries[index].ist = 0; // disabled ist i.e clear 3 bit of ist and 5 bit of reserved field 
    int_entries[index].zero = 0; // set top 32 bit to zero
}



// Function to disable interrupts
void disable_interrupts() {
    asm volatile("cli"); // Clear the interrupt flag
}


// Function to enable interrupts
void enable_interrupts() {
    asm volatile("sti"); // Set the interrupt flag
}


void cpu_exception_handler(registers_t *regs){
    printf("Received Interrupt : %d\n%s\nError Code : %d\nSystem Halted!\n", 
        regs->int_no, exception_messages[regs->int_no], regs->err_code);
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
    asm volatile ("int $0x20");  // Interrupt Request, int no: 32, Timer Interrupt 
    // asm volatile ("int $0x21");  // Interrupt Request, int no : 33, Keyboard Interrupt
    // asm volatile ("int $0x22");  // Interrupt Request, int no: 34
}


void gpf_handler(registers_t *regs){
    printf("recieved interrupt: %d\n", regs->int_no);
    printf("%s\n", exception_messages[regs->int_no]);
    printf("Error Code: %x\n", regs->err_code);
    printf("CS: %x, RIP : %x\n", regs->iret_cs, regs->iret_rip);

    printf("Stack Contents:\n");
    uint64_t *rsp = (uint64_t *)regs->iret_rsp;
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


