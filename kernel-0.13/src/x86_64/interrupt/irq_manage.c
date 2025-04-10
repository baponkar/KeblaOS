/*
Interrupt Request(IRQ)

*/

#include "../../util/util.h"
#include "../../driver/io/ports.h"
#include "../../cpu/cpuid.h"    //has_apic
#include "apic.h" // apic_send_eoi

#include "irq_manage.h"

#define PIC_COMMAND_MASTER 0x20
#define PIC_COMMAND_SLAVE 0xA0
#define PIC_DATA_MASTER 0x21
#define PIC_DATA_SLAVE 0xA1
#define PIC_EOI 0x20 // End of Interrupt

#define TOTAL_IRQ 224   // 256 - 32

void (*irq_routines[TOTAL_IRQ])(registers_t *) = {0};   // This hold all irq routines

void irq_handler(registers_t *regs)
{
    void (*handler)(registers_t *r);    // This is a blank function pointer
    
    int irq_no = regs->int_no - 32;     // Getting IRQ No from Interrupt No
    handler = irq_routines[irq_no];     // Getting Corresponding IRQ Routine

    if (handler){
        handler(regs);
    }

    if(has_apic()){
        apic_send_eoi();
    }else{
        /* If the IDT entry that was invoked was greater than 40
        *  (meaning IRQ8 - 15), then we need to send an EOI to
        *  the slave controller */
        if (regs->int_no >= 40)
        {
            outb(PIC_COMMAND_SLAVE, PIC_EOI); /* slave */
        }

        /* In either case, we need to send an EOI to the master
        *  interrupt controller too */
        outb(PIC_COMMAND_MASTER, PIC_EOI); /* master */
    }
}

// Installing a custom handler function into irq_routines array
void irq_install(int irq_no, void (*handler)(registers_t *r)){
    irq_routines[irq_no] = handler;
}

// Removing handler function from irq_routines array
void irq_uninstall(int irq_no){
    irq_routines[irq_no] = 0;
}
