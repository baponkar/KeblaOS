/*
IPI (Inter-Processor Interrupt) handling for x86_64 architecture.
This code is responsible for sending and handling IPIs between CPU cores in a multi-core system.
*/
#include "../irq_manage.h"      // irq_install, irq_uninstall
#include "apic.h"               // apic_send_eoi, get_lapic_id

#include "../../../lib/stdio.h" // printf

#include "../../../sys/timer/tsc.h"    // tsc_sleep



#include "ipi.h"

uint64_t IPI_VECTOR = 50; // Interrupt vector for IPI (Inter-Processor Interrupt)
uint64_t IPI_IRQ = 18;

void ipi_handler(registers_t *regs) {
    // Handle the IPI interrupt here
    // For example, you can print a message or perform some action
    printf("Received IPI on CPU %d\n", get_lapic_id());
    apic_send_eoi(); // Send EOI to LAPIC after handling the interrupt
}

void init_ipi() {
    // Initialize the IPI handler
    irq_install(IPI_IRQ, &ipi_handler); // Install the IPI handler for IRQ 18
}


