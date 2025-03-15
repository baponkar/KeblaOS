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
#include "../../lib/stdio.h"

#include "pic.h"


#define PIC1_COMMAND_PORT 0x20      //Primary PIC(programmable interrupt controller) Command Port:
#define PIC1_DATA_PORT 0x21         //Primary PIC Data Port
#define PIC2_COMMAND_PORT 0xA0      //Secondary PIC Command Port:
#define PIC2_DATA_PORT 0xA1         //Secondary PIC Data Port

#define PIC_EOI 0x20 // End of Interrupt


// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t *regs)
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

void irq_handler(registers_t *regs)
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





void init_pic_interrupt(){
    asm volatile("cli");

    pic_irq_remap();
   
    asm volatile("sti");
    printf("Successfully PIC Initialized.\n");
}


