/*
SATA(Serial ATA) is a computer bus interface for connecting host bus adapters to 
mass storage devices such as hard disk drives and optical drives. It is a replacement 
for the older Parallel ATA (PATA) standard. SATA has several advantages over PATA, 
including faster data transfer rates, improved reliability, and support for hot swapping 
of drives.
*/

#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../driver/io/ports.h"
#include "../memory/kmalloc.h"
#include "../ahci/ahci.h"
#include "../pci/pci.h"
#include "../x86_64/interrupt/interrupt.h"

#include "sata_disk.h"


#define SATA_DISK_VECTOR_NO 46 // SATA Disk Interrupt Vector Number
#define SATA_DISK_IRQ_NO 14    // SATA Disk IRQ Number

void sata_disk_interrupt_handler(registers_t *regs) {
    // Identify which port triggered the interrupt by checking PxIS registers.
    for (int i = 0; i < num_ports; i++) {
        HBA_PORT_T *port = &abar->ports[i];
        if (port->is & INTERRUPT_OCCURRED) {
            // Process the completion
            // Check for errors or command completion
            // Clear the PxIS register bits
            port->is = port->is;
            // Possibly wake up waiting threads/processes
        }
    }
}


void init_sata_disk(ahci_controller_t *sata_disk) {

    asm volatile("cli"); // Disable interrupts

    interrupt_install_handler(SATA_DISK_IRQ_NO, &sata_disk_interrupt_handler);
    
    asm volatile("sti"); // Enable interrupts

    printf("[INFO] SATA Disk Initialized!\n");
}
















