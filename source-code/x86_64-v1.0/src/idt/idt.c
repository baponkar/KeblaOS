#include "idt.h"

idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;

char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment not present",
    "Stack fault",
    "General protection fault",
    "Page fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Fault",
    "Machine Check", 
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};



void idt_set_gate(uint8_t index, uint64_t offset, uint16_t selector, uint8_t attr){
    
    idt_entries[index].offset_1 = offset & 0xFFFF;
    idt_entries[index].selector = selector;
    idt_entries[index].ist = 0; // disabled ist
    idt_entries[index].type_attributes = attr;
    idt_entries[index].offset_2 = (offset >> 16) & 0xFFFF;
    idt_entries[index].offset_2 = (offset >> 32) & 0xFFFFFFFF;
    idt_entries[index].zero = 0;
}


void isr_install(){
    idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
    idt_ptr.base  = (uint64_t) &idt_entries;

    memset(&idt_entries, 0, sizeof(idt_entry_t) * 256);

    // Setting Interrupts Service Routine Gate(ISR Gate)
    // https://stackoverflow.com/questions/9113310/segment-selector-in-ia-32
   idt_set_gate(  0, (uint64_t)isr0 , 0x08, 0x8E);  // selector = 0x08 = 0b1000, 32-bit Interrupt Gate => attr = 0x8E = 1000 1110, (p=0b1,0b0, dpl=0b00, gate type=0b1110)
   idt_set_gate(  1, (uint64_t)isr1 , 0x08, 0x8E);  // Keyboard
   idt_set_gate(  2, (uint64_t)isr2 , 0x08, 0x8E);  // selector value is 1000 because GDT code segment index is 1
   idt_set_gate(  3, (uint64_t)isr3 , 0x08, 0x8E);  // selector = index + table_to_use + privilege
   idt_set_gate(  4, (uint64_t)isr4 , 0x08, 0x8E);  // selector  = 1<<3(index 1) + 0<<2(TI for GDT 0) + 0<<1(for ring 0) => 1000 + 000 + 00 = 1000 = 0x08
   idt_set_gate(  5, (uint64_t)isr5 , 0x08, 0x8E);
   idt_set_gate(  6, (uint64_t)isr6 , 0x08, 0x8E);
   idt_set_gate(  7, (uint64_t)isr7 , 0x08, 0x8E);
   idt_set_gate(  8, (uint64_t)isr8 , 0x08, 0x8E);
   idt_set_gate(  9, (uint64_t)isr9 , 0x08, 0x8E);
   idt_set_gate( 10, (uint64_t)isr10 , 0x08, 0x8E);
   idt_set_gate( 11, (uint64_t)isr11 , 0x08, 0x8E);
   idt_set_gate( 12, (uint64_t)isr12 , 0x08, 0x8E);
   idt_set_gate( 13, (uint64_t)isr13 , 0x08, 0x8E);
   idt_set_gate( 14, (uint64_t)isr14 , 0x08, 0x8E); // paging
   idt_set_gate( 15, (uint64_t)isr15 , 0x08, 0x8E);
   idt_set_gate( 16, (uint64_t)isr16 , 0x08, 0x8E);
   idt_set_gate( 17, (uint64_t)isr17 , 0x08, 0x8E);
   idt_set_gate( 18, (uint64_t)isr18 , 0x08, 0x8E);
   idt_set_gate( 19, (uint64_t)isr19 , 0x08, 0x8E);
   idt_set_gate( 20, (uint64_t)isr20 , 0x08, 0x8E);
   idt_set_gate( 21, (uint64_t)isr21 , 0x08, 0x8E);
   idt_set_gate( 22, (uint64_t)isr22 , 0x08, 0x8E);
   idt_set_gate( 23, (uint64_t)isr23 , 0x08, 0x8E);
   idt_set_gate( 24, (uint64_t)isr24 , 0x08, 0x8E);
   idt_set_gate( 25, (uint64_t)isr25 , 0x08, 0x8E);
   idt_set_gate( 26, (uint64_t)isr26 , 0x08, 0x8E);
   idt_set_gate( 27, (uint64_t)isr27 , 0x08, 0x8E);
   idt_set_gate( 28, (uint64_t)isr28 , 0x08, 0x8E);
   idt_set_gate( 29, (uint64_t)isr29 , 0x08, 0x8E);
   idt_set_gate( 30, (uint64_t)isr30 , 0x08, 0x8E);
   idt_set_gate( 31, (uint64_t)isr31 , 0x08, 0x8E);

   idt_set_gate(128, (uint64_t)isr128, 0x08, 0x8E); //System call Write
   idt_set_gate(177, (uint64_t)isr177, 0x08, 0x8E); //System call Read

   idt_flush((uint64_t) &idt_ptr);
}

// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t regs)
{
    if(regs.int_no == 128){
        // syscall_handler(&regs);
        //return;
    }
    else if(regs.int_no == 177){
        // syscall_handler(&regs);
        //return;
    }
    else if (regs.int_no == 14) { // Check if it is a page fault
        // page_fault(&regs); // Call your page fault handler directly
        //return;
    }
    else if(regs.int_no < 32){
        print("recieved interrupt: ");
        print_dec(regs.int_no);
        putchar('\n');
        print(exception_messages[regs.int_no]);
        putchar('\n');
        print("System Halted!\n");
        for (;;);
    }
}

/* This array is actually an array of function pointers. We use
*  this to handle custom Interrupt handlers for a given Interrupt */
void *interrupt_routines[16] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};



/* This installs a custom Interrupt handler for the given Interrupt */
void interrupt_install_handler(int int_no, void (*handler)(registers_t *r))
{
    interrupt_routines[int_no] = handler;
}


/* This clears the handler for a given Interrupt */
void interrupt_uninstall_handler(int int_no)
{
    interrupt_routines[int_no] = 0;
}

void init_idt(){
    isr_install();
}

void test_interrupt() {
    // Testing Interrupts
     asm volatile ("int $0x3");    //  Breakpoint
    // asm volatile ("int $0x0");     // Division By Zero
    // asm volatile ("int $0x20");     // Interrupt Request 32
    // print_dec( 104 / 0 );
    // asm volatile ("int $0xE");       // Page Fault
}