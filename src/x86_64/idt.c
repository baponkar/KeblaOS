/*
Interrupt Descriptor Table
https://wiki.osdev.org/Interrupt_Descriptor_Table
https://stackoverflow.com/questions/52214531/x86-64-order-of-passing-parameters-in-registers
https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/05_InterruptHandling.md
https://web.archive.org/web/20160326064709/http://jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html

*/

#include "idt.h"

idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;

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
    "Reserved", // 19
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



void idt_set_gate(uint8_t index, uint64_t offset, uint16_t selector, uint8_t attr){
    idt_entries[index].offset_1 = (uint16_t) offset & 0xFFFF; // set lower 16 bit
    idt_entries[index].offset_2 = (uint16_t) (offset >> 16) & 0xFFFF; // set 16 bit
    idt_entries[index].offset_3 = (uint32_t) (offset >> 32) & 0xFFFFFFFF; // set upper 32 bit

    idt_entries[index].selector = selector;                   // set 16 bit of selector
    //              |P|DPL|R|TYPE|
    // for x86_64 : |1|00 |0|1110| ==> 10001110 ==> 0x8E
    idt_entries[index].type_attributes = attr;    // set 8 bit  of P(1 bit) + DPL(2 bit) + gate type(4 bit) + 0(1 bit)
    
    idt_entries[index].ist = 0; // disabled ist i.e clear 3 bit of ist and 5 bit of reserved field 
    idt_entries[index].zero = 0; // set top 32 bit to zero
}


void isr_install(){
    idt_ptr.limit = (sizeof(idt_entry_t) * 256) - 1;
    idt_ptr.base  = (uint64_t) &idt_entries;

    // for safety clearing memories
    memset(&idt_entries, 0, sizeof(idt_entry_t) * 256);

   // Setting Interrupts Service Routine Gate(ISR Gate)
   // https://stackoverflow.com/questions/9113310/segment-selector-in-ia-32
idt_set_gate(  0, (uint64_t)isr0 , 0x0008, 0x8E);  // selector = 0x08 = 0b1000, 64-bit Interrupt Gate => attr = 0x8E = 1000 1110, (p=0b1,0b0, dpl=0b00, gate type=0b1110)
   idt_set_gate(  1, (uint64_t)isr1 , 0x0008, 0x8E);
   idt_set_gate(  2, (uint64_t)isr2 , 0x0008, 0x8E);  // selector value is 1000 because GDT code segment index is 1
   idt_set_gate(  3, (uint64_t)isr3 , 0x0008, 0x8E);  // selector = index + table_to_use + privilege
   idt_set_gate(  4, (uint64_t)isr4 , 0x0008, 0x8E);  // selector  = 1<<3(index 1) + 0<<2(TI for GDT 0) + 0<<1(for ring 0) => 1000 + 000 + 00 = 1000 = 0x08
   idt_set_gate(  5, (uint64_t)isr5 , 0x0008, 0x8E);
   idt_set_gate(  6, (uint64_t)isr6 , 0x0008, 0x8E);
   idt_set_gate(  7, (uint64_t)isr7 , 0x0008, 0x8E);
   idt_set_gate(  8, (uint64_t)isr8 , 0x0008, 0x8E);
   idt_set_gate(  9, (uint64_t)isr9 , 0x0008, 0x8E);
   idt_set_gate( 10, (uint64_t)isr10 , 0x0008, 0x8E);
   idt_set_gate( 11, (uint64_t)isr11 , 0x0008, 0x8E);
   idt_set_gate( 12, (uint64_t)isr12 , 0x0008, 0x8E);
   idt_set_gate( 13, (uint64_t)isr13 , 0x0008, 0x8E);
   idt_set_gate( 14, (uint64_t)isr14 , 0x0008, 0x8E); // paging
   idt_set_gate( 15, (uint64_t)isr15 , 0x0008, 0x8E);
   idt_set_gate( 16, (uint64_t)isr16 , 0x0008, 0x8E);
   idt_set_gate( 17, (uint64_t)isr17 , 0x0008, 0x8E);
   idt_set_gate( 18, (uint64_t)isr18 , 0x0008, 0x8E);
   idt_set_gate( 19, (uint64_t)isr19 , 0x0008, 0x8E);
   idt_set_gate( 20, (uint64_t)isr20 , 0x0008, 0x8E);
   idt_set_gate( 21, (uint64_t)isr21 , 0x0008, 0x8E);
   idt_set_gate( 22, (uint64_t)isr22 , 0x0008, 0x8E);
   idt_set_gate( 23, (uint64_t)isr23 , 0x0008, 0x8E);
   idt_set_gate( 24, (uint64_t)isr24 , 0x0008, 0x8E);
   idt_set_gate( 25, (uint64_t)isr25 , 0x0008, 0x8E);
   idt_set_gate( 26, (uint64_t)isr26 , 0x0008, 0x8E);
   idt_set_gate( 27, (uint64_t)isr27 , 0x0008, 0x8E);
   idt_set_gate( 28, (uint64_t)isr28 , 0x0008, 0x8E);
   idt_set_gate( 29, (uint64_t)isr29 , 0x0008, 0x8E);
   idt_set_gate( 30, (uint64_t)isr30 , 0x0008, 0x8E);
   idt_set_gate( 31, (uint64_t)isr31 , 0x0008, 0x8E);
    
   idt_set_gate(128, (uint64_t)isr128, 0x0008, 0x8E); //System call Write
   idt_set_gate(177, (uint64_t)isr177, 0x0008, 0x8E); //System call Read
}


// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t regs)
{
    if(regs.int_no == 128){
        print("Interrupt 128\n");
        // syscall_handler(&regs);
        return;
    }
    else if(regs.int_no == 177){
        print("Interrupt 177\n");
        // syscall_handler(&regs);
        return;
    }
    else if (regs.int_no == 14) { // Check if it is a page fault
        print("Interrupt 14\n");
        // page_fault(&regs); // Call your page fault handler directly
        return;
    }
    else if(regs.int_no < 32){
        print("recieved interrupt: ");
        print_dec(regs.int_no);
        putchar('\n');
        print(exception_messages[regs.int_no]);
        putchar('\n');
        print("Error Code: ");
        print_dec(regs.err_code);
        print("\n");
        print("System Halted!\n");
        hcf();
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
    idt_flush((uint64_t) &idt_ptr);
    irq_remap();
    irq_install();
    enable_interrupts();
    print("Successfully IDT Enabled!\n");
}


void test_interrupt() {
    print("Testing Interrupts\n");
    // asm volatile ("div %b0" :: "a"(0)); // Int no 0
    // asm volatile ("int $0x3");   // Breakpoint int no : 3
    // asm volatile ("int $0x0");   // Division By Zero, int no : 0
    // asm volatile ("int $0xE");   // Page Fault Request, int no: 14
    // asm volatile("int $0xF");    // int no 15
    // asm volatile("int $0x10");   // int no 16
    // asm volatile("int $0x11");   // int no 17
    // asm volatile ("int $0x20");  // Interrupt Request, int no: 32 
    // asm volatile ("int $0x21");     // Interrupt Request, int no : 33
    // asm volatile ("int $0x22");  // Interrupt Request, int no: 34
}

#define PIC1_COMMAND_PORT 0x20      //Primary PIC(programmable interrupt controller) Command Port:
#define PIC1_DATA_PORT 0x21         //Primary PIC Data Port
#define PIC2_COMMAND_PORT 0xA0      //Secondary PIC Command Port:
#define PIC2_DATA_PORT 0xA1         //Secondary PIC Data Port

void irq_remap(void)
{
    outb(PIC1_COMMAND_PORT, 0x11);  // 0x11 is the command to initialize the PICs in cascade mode, 
    outb(PIC2_COMMAND_PORT, 0x11);  // which means the two PICs will be working together

    outb(PIC1_DATA_PORT, 0x20);     // For PIC1 (master), the vector base is set to 0x20 (interrupts 32 to 39).
    outb(PIC2_DATA_PORT, 0x28);     // For PIC2 (slave), the vector base is set to 0x28 (interrupts 40 to 47).

    outb(PIC1_DATA_PORT, 0x04);     // For the master PIC, 0x04 means that the slave PIC is connected to IRQ2.
    outb(PIC2_DATA_PORT, 0x02);     // For the slave PIC, 0x02 indicates that it's connected to the master PIC's IRQ2.

    outb(PIC1_DATA_PORT, 0x01);     // 0x01 enables the 8086 mode (which is the typical mode for x86 systems).
    outb(PIC2_DATA_PORT, 0x01);

    outb(PIC1_DATA_PORT, 0x0);      // These commands unmask all interrupts on the master and slave PICs by 
    outb(PIC2_DATA_PORT, 0x0);      // setting the interrupt mask to 0x0, meaning all IRQ lines are enabled.
}


void irq_install()
{
    // Setup for Interrupts Request Gate (IRQ Gate)
    idt_set_gate(32, (uint64_t)irq0, 0x0008, 0x8E); // Timer Interrupt
    idt_set_gate(33, (uint64_t)irq1, 0x08, 0x8E);
    idt_set_gate(34, (uint64_t)irq2, 0x08, 0x8E);
    idt_set_gate(35, (uint64_t)irq3, 0x08, 0x8E);
    idt_set_gate(36, (uint64_t)irq4, 0x08, 0x8E);
    idt_set_gate(37, (uint64_t)irq5, 0x08, 0x8E);
    idt_set_gate(38, (uint64_t)irq6, 0x08, 0x8E);
    idt_set_gate(39, (uint64_t)irq7, 0x08, 0x8E);
    idt_set_gate(40, (uint64_t)irq8, 0x08, 0x8E);
    idt_set_gate(41, (uint64_t)irq9, 0x08, 0x8E);
    idt_set_gate(42, (uint64_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint64_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint64_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint64_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint64_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint64_t)irq15, 0x08, 0x8E);
}

#define PIC_EOI 0x20 // End of Interrupt

void irq_handler(registers_t *r)
{
    /* This is a blank function pointer */
    void (*handler)(registers_t *r);
    
    /* Find out if we have a custom handler to run for this
    *  IRQ, and then finally, run it */
    handler = interrupt_routines[r->int_no - 32];
    if (handler)
    {
        handler(r);
    }

    /* If the IDT entry that was invoked was greater than 40
    *  (meaning IRQ8 - 15), then we need to send an EOI to
    *  the slave controller */
    if (r->int_no >= 40)
    {
        outb(PIC2_COMMAND_PORT, PIC_EOI); /* slave */
    }

    /* In either case, we need to send an EOI to the master
    *  interrupt controller too */
    outb(PIC1_COMMAND_PORT, PIC_EOI); /* master */
}




