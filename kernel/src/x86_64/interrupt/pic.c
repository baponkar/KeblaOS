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
#include "../../lib/stdio.h"

#include "pic.h"


#define PIC1_COMMAND_PORT 0x20      //Primary PIC(programmable interrupt controller) Command Port:
#define PIC1_DATA_PORT 0x21         //Primary PIC Data Port
#define PIC2_COMMAND_PORT 0xA0      //Secondary PIC Command Port:
#define PIC2_DATA_PORT 0xA1         //Secondary PIC Data Port

#define PIC_EOI 0x20 // End of Interrupt

#define ICW_1 0x11
#define ICW_2_M 0x20
#define ICW_2_S 0x28
#define ICW_3_M 0x04
#define ICW_3_S 0x02
#define ICW_4 0x01


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


// Function to disable the PIC
void disable_pic() {
    outb(PIC1_COMMAND_PORT, ICW_1); // ICW1: Select 8086 mode
    outb(PIC2_COMMAND_PORT, ICW_1); // ICW1: Select 8086 mode
    outb(PIC1_DATA_PORT, ICW_2_M);  // ICW2: Master PIC vector offset
    outb(PIC2_DATA_PORT, ICW_2_S);  // ICW2: Slave PIC vector offset
    outb(PIC1_DATA_PORT, ICW_3_M);  // ICW3: Tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    outb(PIC2_DATA_PORT, ICW_3_S);  // ICW3: Tell Slave PIC its cascade identity (0000 0010)
    outb(PIC1_DATA_PORT, ICW_4);    // ICW4: 8086 mode, normal EOI
    outb(PIC2_DATA_PORT, ICW_4);    // ICW4: 8086 mode, normal EOI
    outb(PIC1_DATA_PORT, 0xFF);     // Mask all interrupts
    outb(PIC2_DATA_PORT, 0xFF);     // Mask all interrupts

    printf("[Info] PIC Disabled.\n");
}


void init_pic_interrupt(){
    asm volatile("cli");

    pic_irq_remap();
   
    asm volatile("sti");
    printf("[Info] Successfully PIC Initialized.\n");
}


