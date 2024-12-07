
#include  "idt.h"


idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;


void init_idt()
{
   idt_ptr.limit = sizeof(idt_entry_t) * 256 -1;
   idt_ptr.base  = (uint32_t) &idt_entries;

   memset(&idt_entries, 0, sizeof(idt_entry_t) * 256);

   // Setting Interrupts Service Routine Gate(ISR Gate)
   idt_set_gate(  0, (uint32_t)isr0 , 0x08, 0x8E); // selector = 0x08 = 1000
   idt_set_gate(  1, (uint32_t)isr1 , 0x08, 0x8E); // 32-bit Interrupt Gate => flags = 0x8E = 1000 1110, (p=1, dpl=0b00, gate type=0b1110)
   idt_set_gate(  2, (uint32_t)isr2 , 0x08, 0x8E);
   idt_set_gate(  3, (uint32_t)isr3 , 0x08, 0x8E);
   idt_set_gate(  4, (uint32_t)isr4 , 0x08, 0x8E);
   idt_set_gate(  5, (uint32_t)isr5 , 0x08, 0x8E);
   idt_set_gate(  6, (uint32_t)isr6 , 0x08, 0x8E);
   idt_set_gate(  7, (uint32_t)isr7 , 0x08, 0x8E);
   idt_set_gate(  8, (uint32_t)isr8 , 0x08, 0x8E);
   idt_set_gate(  9, (uint32_t)isr9 , 0x08, 0x8E);
   idt_set_gate( 10, (uint32_t)isr10 , 0x08, 0x8E);
   idt_set_gate( 11, (uint32_t)isr11 , 0x08, 0x8E);
   idt_set_gate( 12, (uint32_t)isr12 , 0x08, 0x8E);
   idt_set_gate( 13, (uint32_t)isr13 , 0x08, 0x8E);
   idt_set_gate( 14, (uint32_t)isr14 , 0x08, 0x8E);
   idt_set_gate( 15, (uint32_t)isr15 , 0x08, 0x8E);
   idt_set_gate( 16, (uint32_t)isr16 , 0x08, 0x8E);
   idt_set_gate( 17, (uint32_t)isr17 , 0x08, 0x8E);
   idt_set_gate( 18, (uint32_t)isr18 , 0x08, 0x8E);
   idt_set_gate( 19, (uint32_t)isr19 , 0x08, 0x8E);
   idt_set_gate( 20, (uint32_t)isr20 , 0x08, 0x8E);
   idt_set_gate( 21, (uint32_t)isr21 , 0x08, 0x8E);
   idt_set_gate( 22, (uint32_t)isr22 , 0x08, 0x8E);
   idt_set_gate( 23, (uint32_t)isr23 , 0x08, 0x8E);
   idt_set_gate( 24, (uint32_t)isr24 , 0x08, 0x8E);
   idt_set_gate( 25, (uint32_t)isr25 , 0x08, 0x8E);
   idt_set_gate( 26, (uint32_t)isr26 , 0x08, 0x8E);
   idt_set_gate( 27, (uint32_t)isr27 , 0x08, 0x8E);
   idt_set_gate( 28, (uint32_t)isr28 , 0x08, 0x8E);
   idt_set_gate( 29, (uint32_t)isr29 , 0x08, 0x8E);
   idt_set_gate( 30, (uint32_t)isr30 , 0x08, 0x8E);
   idt_set_gate( 31, (uint32_t)isr31 , 0x08, 0x8E);

   idt_set_gate(128, (uint32_t)isr128, 0x08, 0x8E); //System calls
   idt_set_gate(177, (uint32_t)isr177, 0x08, 0x8E); //System calls

   idt_flush((uint32_t) &idt_ptr);
}



void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
   idt_entries[num].base_lo = base & 0xFFFF;
   idt_entries[num].base_hi = (base >> 16) & 0xFFFF;

   idt_entries[num].sel     = sel;
   idt_entries[num].always0 = 0;
   // We must uncomment the OR below when we get to using user-mode.
   // It sets the interrupt gate's privilege level to 3.
   idt_entries[num].flags   = flags /* | 0x60 */;
}


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




// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t regs)
{
   if(regs.int_no == 128){
        //syscall_handler(&regs);
    }
    else if (regs.int_no == 14) { // Check if it is a page fault
        disable_cursor();
        page_fault_handler(&regs); // Call your page fault handler directly
        return;
    }
    else if(regs.int_no < 32){
        disable_cursor();
        errprint("recieved interrupt: ");
        print_dec(regs.int_no);
        putchar('\n');
        errprint(exception_messages[regs.int_no]);
        putchar('\n');
        errprint("System Halted!\n");
        for (;;);
    }
}



/* This array is actually an array of function pointers. We use
*  this to handle custom IRQ handlers for a given IRQ */
void *irq_routines[16] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};



/* This installs a custom IRQ handler for the given IRQ */
void irq_install_handler(int irq, void (*handler)(registers_t *r))
{
    irq_routines[irq] = handler;
}


/* This clears the handler for a given IRQ */
void irq_uninstall_handler(int irq)
{
    irq_routines[irq] = 0;
}



/* Normally, IRQs 0 to 7 are mapped to entries 8 to 15. This
*  is a problem in protected mode, because IDT entry 8 is a
*  Double Fault! Without remapping, every time IRQ0 fires,
*  you get a Double Fault Exception, which is NOT actually
*  what's happening. We send commands to the Programmable
*  Interrupt Controller (PICs - also called the 8259's) in
*  order to make IRQ0 to 15 be remapped to IDT entries 32 to
*  47 */
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




/* We first remap the interrupt controllers, and then we install
*  the appropriate ISRs to the correct entries in the IDT. This
*  is just like installing the exception handlers */
void irq_install()
{
    irq_remap();

    // Setup for Interrupts Request Gate (IRQ Gate)
    idt_set_gate(32, (unsigned)irq0, 0x08, 0x8E);
    idt_set_gate(33, (unsigned)irq1, 0x08, 0x8E);
    idt_set_gate(34, (unsigned)irq2, 0x08, 0x8E);
    idt_set_gate(35, (unsigned)irq3, 0x08, 0x8E);
    idt_set_gate(36, (unsigned)irq4, 0x08, 0x8E);
    idt_set_gate(37, (unsigned)irq5, 0x08, 0x8E);
    idt_set_gate(38, (unsigned)irq6, 0x08, 0x8E);
    idt_set_gate(39, (unsigned)irq7, 0x08, 0x8E);
    idt_set_gate(40, (unsigned)irq8, 0x08, 0x8E);
    idt_set_gate(41, (unsigned)irq9, 0x08, 0x8E);
    idt_set_gate(42, (unsigned)irq10, 0x08, 0x8E);
    idt_set_gate(43, (unsigned)irq11, 0x08, 0x8E);
    idt_set_gate(44, (unsigned)irq12, 0x08, 0x8E);
    idt_set_gate(45, (unsigned)irq13, 0x08, 0x8E);
    idt_set_gate(46, (unsigned)irq14, 0x08, 0x8E);
    idt_set_gate(47, (unsigned)irq15, 0x08, 0x8E);
}



/* Each of the IRQ ISRs point to this function, rather than
*  the 'fault_handler' in 'isrs.c'. The IRQ Controllers need
*  to be told when you are done servicing them, so you need
*  to send them an "End of Interrupt" command (0x20). There
*  are two 8259 chips: The first exists at 0x20, the second
*  exists at 0xA0. If the second controller (an IRQ from 8 to
*  15) gets an interrupt, you need to acknowledge the
*  interrupt at BOTH controllers, otherwise, you only send
*  an EOI command to the first controller. If you don't send
*  an EOI, you won't raise any more IRQs */
void irq_handler(registers_t *r)
{
    /* This is a blank function pointer */
    void (*handler)(registers_t *r);

    /* Find out if we have a custom handler to run for this
    *  IRQ, and then finally, run it */
    handler = irq_routines[r->int_no - 32];
    if (handler)
    {
        handler(r);
    }

    /* If the IDT entry that was invoked was greater than 40
    *  (meaning IRQ8 - 15), then we need to send an EOI to
    *  the slave controller */
    if (r->int_no >= 40)
    {
        outb(PIC2_COMMAND_PORT, 0x20); /* slave */
    }

    /* In either case, we need to send an EOI to the master
    *  interrupt controller too */
    outb(PIC1_COMMAND_PORT, 0x20); /* master */
}


