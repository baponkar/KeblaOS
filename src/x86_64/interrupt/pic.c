/*
Interrupt Descriptor Table
https://wiki.osdev.org/Interrupt_Descriptor_Table
https://stackoverflow.com/questions/52214531/x86-64-order-of-passing-parameters-in-registers
https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/05_InterruptHandling.md
https://web.archive.org/web/20160326064709/http://jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html
https://stackoverflow.com/questions/79386685/how-does-stack-memory-will-be-use-to-store-cpu-state-during-interrupt?noredirect=1#comment140001157_79386685

*/

#include "interrupt.h"

#include "../../driver/io/ports.h"
#include "../../mmu/paging.h"
#include "../../lib/string.h"
#include "../../lib/stdio.h"

#include "pic.h"


#define PIC1_COMMAND_PORT 0x20      //Primary PIC(programmable interrupt controller) Command Port:
#define PIC1_DATA_PORT 0x21         //Primary PIC Data Port
#define PIC2_COMMAND_PORT 0xA0      //Secondary PIC Command Port:
#define PIC2_DATA_PORT 0xA1         //Secondary PIC Data Port

#define PIC_EOI 0x20 // End of Interrupt


// This gets called from our ASM interrupt handler stub.
void pic_isr_handler(registers_t *regs)
{
    if(regs->int_no == 128){
        printf("Received Interrupt : %d\n", regs->int_no);
        // syscall_handler(&regs);
        return;
    }else if(regs->int_no == 177){
        printf("Received Interrupt : %d\n", regs->int_no);
        // syscall_handler(&regs);
        return;
    }else if (regs->int_no == 14) {
        page_fault_handler(regs);
        return;
    }else if(regs->int_no == 13){
        // print("General Protection Fault\n");
        // debug_error_code(regs->err_code);
        gpf_handler(regs);
        return;
    }else if(regs->int_no < 32){
        printf("Received Interrupt : %d\n%s\nError Code : %d\nSystem Halted!\n", 
            regs->int_no, exception_messages[regs->int_no], regs->err_code);
        // debug_error_code(regs->err_code);
        halt_kernel();
    }else{
        printf("Received Interrupt : %d\n", regs->int_no);
        return;
    }
}


void pic_isr_install(){

   // Setting Interrupts Service Routine Gate(ISR Gate)
   // https://stackoverflow.com/questions/9113310/segment-selector-in-ia-32
   int_set_gate(  0, (uint64_t)&pic_isr0 ,  0x8, 0x8E);  // selector = 0x08 = 0b1000, 64-bit Interrupt Gate => attr = 0x8E = 1 0 00 1110, (p=0b1,0b0, dpl=0b00, gate type=0b1110)
   int_set_gate(  1, (uint64_t)&pic_isr1 ,  0x8, 0x8E);
   int_set_gate(  2, (uint64_t)&pic_isr2 ,  0x8, 0x8E);  // selector value is 1000 because GDT code segment index is 1
   int_set_gate(  3, (uint64_t)&pic_isr3 ,  0x8, 0x8E);  // selector = index + table_to_use + privilege
   int_set_gate(  4, (uint64_t)&pic_isr4 ,  0x8, 0x8E);  // selector  = 1<<3(index 1) + 0<<2(TI for GDT 0) + 0<<1(for ring 0) => 1000 + 000 + 00 = 1000 = 0x08
   int_set_gate(  5, (uint64_t)&pic_isr5 ,  0x8, 0x8E);
   int_set_gate(  6, (uint64_t)&pic_isr6 ,  0x8, 0x8E);
   int_set_gate(  7, (uint64_t)&pic_isr7 ,  0x8, 0x8E);
   int_set_gate(  8, (uint64_t)&pic_isr8 ,  0x8, 0x8E);
   int_set_gate(  9, (uint64_t)&pic_isr9 ,  0x8, 0x8E);
   int_set_gate( 10, (uint64_t)&pic_isr10 , 0x8, 0x8E);
   int_set_gate( 11, (uint64_t)&pic_isr11 , 0x8, 0x8E);
   int_set_gate( 12, (uint64_t)&pic_isr12 , 0x8, 0x8E);
   int_set_gate( 13, (uint64_t)&pic_isr13 , 0x8, 0x8E);
   int_set_gate( 14, (uint64_t)&pic_isr14 , 0x8, 0x8E); // paging
   int_set_gate( 15, (uint64_t)&pic_isr15 , 0x8, 0x8E);

   int_set_gate( 16, (uint64_t)&pic_isr16 , 0x8, 0x8E);
   int_set_gate( 17, (uint64_t)&pic_isr17 , 0x8, 0x8E);
   int_set_gate( 18, (uint64_t)&pic_isr18 , 0x8, 0x8E);
   int_set_gate( 19, (uint64_t)&pic_isr19 , 0x8, 0x8E);
   int_set_gate( 20, (uint64_t)&pic_isr20 , 0x8, 0x8E);
   int_set_gate( 21, (uint64_t)&pic_isr21 , 0x8, 0x8E);
   int_set_gate( 22, (uint64_t)&pic_isr22 , 0x8, 0x8E);
   int_set_gate( 23, (uint64_t)&pic_isr23 , 0x8, 0x8E);
   int_set_gate( 24, (uint64_t)&pic_isr24 , 0x8, 0x8E);
   int_set_gate( 25, (uint64_t)&pic_isr25 , 0x8, 0x8E);
   int_set_gate( 26, (uint64_t)&pic_isr26 , 0x8, 0x8E);
   int_set_gate( 27, (uint64_t)&pic_isr27 , 0x8, 0x8E);
   int_set_gate( 28, (uint64_t)&pic_isr28 , 0x8, 0x8E);
   int_set_gate( 29, (uint64_t)&pic_isr29 , 0x8, 0x8E);
   int_set_gate( 30, (uint64_t)&pic_isr30 , 0x8, 0x8E);
   int_set_gate( 31, (uint64_t)&pic_isr31 , 0x8, 0x8E);
    
   int_set_gate(128, (uint64_t)&pic_isr128, 0x8, 0x8E); //System call Write
   int_set_gate(177, (uint64_t)&pic_isr177, 0x8, 0x8E); //System call Read
}



void pic_irq_handler(registers_t *regs)
{
    /* This is a blank function pointer */
    void (*handler)(registers_t *r);
    
    /* Find out if we have a custom handler to run for this
    *  IRQ, and then finally, run it */
    handler = interrupt_routines[regs->int_no - 32];

    if (handler)
    {
        handler(regs);
    }

    /* If the IDT entry that was invoked was greater than 40
    *  (meaning IRQ8 - 15), then we need to send an EOI to
    *  the slave controller */
    if (regs->int_no >= 40)
    {
        outb(PIC2_COMMAND_PORT, PIC_EOI); /* slave */
    }

    /* In either case, we need to send an EOI to the master
    *  interrupt controller too */
    outb(PIC1_COMMAND_PORT, PIC_EOI); /* master */
}


void pic_irq_remap()
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


void pic_irq_install()
{
    int_set_gate(32, (uint64_t)&pic_irq0, 0x08, 0x8E);  // Timer Interrupt
    int_set_gate(33, (uint64_t)&pic_irq1, 0x08, 0x8E);  // Keyboard Interrupt
    int_set_gate(34, (uint64_t)&pic_irq2, 0x08, 0x8E);  // Cascade (for PIC chaining)
    int_set_gate(35, (uint64_t)&pic_irq3, 0x08, 0x8E);  // COM2 (Serial Port 2)
    int_set_gate(36, (uint64_t)&pic_irq4, 0x08, 0x8E);  // COM1 (Serial Port 1)
    int_set_gate(37, (uint64_t)&pic_irq5, 0x08, 0x8E);  // LPT2 (Parallel Port 2) or Sound Card
    int_set_gate(38, (uint64_t)&pic_irq6, 0x08, 0x8E);  // Floppy Disk Controller
    int_set_gate(39, (uint64_t)&pic_irq7, 0x08, 0x8E);  // LPT1 (Parallel Port 1) / Spurious IRQ
    int_set_gate(40, (uint64_t)&pic_irq8, 0x08, 0x8E);  // Real-Time Clock (RTC)
    int_set_gate(41, (uint64_t)&pic_irq9, 0x08, 0x8E);  // ACPI / General system use
    int_set_gate(42, (uint64_t)&pic_irq10, 0x08, 0x8E); // Available (often used for SCSI or NIC)
    int_set_gate(43, (uint64_t)&pic_irq11, 0x08, 0x8E); // Available (often used for PCI devices)
    int_set_gate(44, (uint64_t)&pic_irq12, 0x08, 0x8E); // PS/2 Mouse
    int_set_gate(45, (uint64_t)&pic_irq13, 0x08, 0x8E); // FPU / Floating-Point Unit (Coprocessor)
    int_set_gate(46, (uint64_t)&pic_irq14, 0x08, 0x8E); // Primary ATA Hard Disk Controller
    int_set_gate(47, (uint64_t)&pic_irq15, 0x08, 0x8E); // Secondary ATA Hard Disk Controller
}


void init_pic_interrupt(){
    disable_interrupts();

    int_ptr.limit = (sizeof(int_entry_t) * 256) - 1;
    int_ptr.base  = (uint64_t) &int_entries;

    // for safety clearing memories
    memset((void *)&int_entries, 0, (size_t) (sizeof(int_entry_t) * 256));
    interrupt_flush((uint64_t) &int_ptr);

    pic_isr_install();

    pic_irq_remap();
    pic_irq_install();

    enable_interrupts();
    printf("Successfully PIC Initialized.\n");
}


