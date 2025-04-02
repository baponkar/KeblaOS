/*

AHCI  : Advance Host Controller Interface - is developed by Intel to facilitate handling SATA devices. 

ATA   : Advanced Technology Attachment
ATAPI : Advanced Technology Attachment Packet Interface (Serial) - Used for most modern optical drives.
ATAPI : Advanced Technology Attachment Packet Interface (Parallel) - Commonly used for optical drives.

PCI  : Peripheral Component Interconnect
PATA : Parallel Advanced Technology Attachment
SATA : Serial Advanced Technology Attachment
HBA  :  Host Base Address
FIS  : Frame Information Structure
GHCR : Global Host Control Register
PRD  : Physical Region Descriptor
PRDT : Physical Region Descriptor Table
DMA  : Direct Memory Access - DMA is a technology that allows data to be transferred directly 
       between system memory (RAM) and a device (e.g., a SATA hard drive or SSD) 
       without involving the CPU for every byte of data. This significantly improves 
       performance by reducing CPU overhead.
PIO  : Programmed Input/Output
BIST : Built-In Self-Test
IDE  : Integrated Drive Electronics - IDE registers are used for communication between the CPU and storage devices.
LBA  : Logical Block Addressing - LBA is a method used to specify the location of data blocks on a storage device 
       such as a hard disk or SSD.
NCQ  : Native Command Queuing.


References:
    https://wiki.osdev.org/AHCI
    https://wiki.osdev.org/SATA
*/

#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../driver/io/ports.h"
#include "../memory/kmalloc.h"
#include "../memory/vmm.h"
#include "../memory/kheap.h"
#include "../disk/disk.h"

#include "ahci.h"

#define HBA_GHC_AHCI_ENABLE (1 << 31)   // 0x80000000 
#define HBA_PORT_CMD_ST     (1 << 0)    // 0x1
#define HBA_PORT_CMD_FRE    (1 << 4)    // 0x10
#define HBA_PORT_CMD_CR     (1 << 15)   // 0x8000

#define ATA_CMD_READ_DMA_EX 0x25
#define ATA_CMD_WRITE_DMA_EX 0x35


ahci_controller_t sata_disk = {0};


void ahci_init() {
    hba_mem *hba = (hba_mem*)(sata_disk.abar);
    
    // Enable AHCI in GHC
    hba->ghc |= HBA_GHC_AHCI_ENABLE;
    while (!(hba->ghc & HBA_GHC_AHCI_ENABLE)); // Wait for enable

    // Find active port
    uint32_t pi = hba->pi;
    for (int i = 0; i < 32; i++) {
        if (pi & (1 << i)) {
            struct hba_port *port = &hba->ports[i];
            uint32_t ssts = port->ssts;
            if ((ssts & 0xF) == 3) { // Device detected
                ahci_port_init(port);
                sata_disk.initialized = true;
                return;
            }
        }
    }
    printf("AHCI: No active port found\n");
}


void ahci_port_init(struct hba_port *port) {
    // Stop command engine
    port->cmd &= ~HBA_PORT_CMD_ST;
    while (port->cmd & HBA_PORT_CMD_CR);

    // Allocate command list and FIS
    // Assuming aligned allocation functions exist
    void *cmd_list = (void *) kmalloc_a(1024, 1); // 32 entries * 32 bytes
    printf("cmd_list: %x\n", (uint64_t)cmd_list);
    void *fis = (void *) kmalloc_a(256, 1);

    // Set command list and FIS base
    port->clb = (uint32_t)(uintptr_t) (uint64_t)cmd_list;
    port->clbu = (uint32_t)((uintptr_t) (uint64_t)cmd_list) >> 32;
    port->fb = (uint32_t)(uintptr_t) (uint64_t)fis;
    port->fbu = (uint32_t)((uintptr_t)(uint64_t)fis >> 32);

    // Enable FIS and start engine
    port->cmd |= HBA_PORT_CMD_FRE;
    port->cmd |= HBA_PORT_CMD_ST;
}



int ahci_read(uint64_t lba, uint32_t count, void *buffer) {
    if (!sata_disk.initialized) return -1;
    hba_mem *hba = (hba_mem*)sata_disk.abar;
    struct hba_port *port = &hba->ports[0]; // Assuming port 0

    // Prepare command header
    volatile hba_cmd_header *cmdheader = (volatile hba_cmd_header *)(uintptr_t)port->clb;;     // Access command list
    cmdheader->cfl = sizeof(h2d_fis)/4;     // Size in DWORDS
    cmdheader->w = 0;                       // Read
    cmdheader->prdtl = 1;                   // 1 PRDT entry

    // Setup command table
    hba_cmd_table *cmdtbl = (hba_cmd_table *) kmalloc_a(sizeof(hba_cmd_table), 1);
    memset(cmdtbl, 0, sizeof(hba_cmd_table));

    // In ahci_read():
    // After allocating cmdtbl, add:
    uint64_t phys_cmdtbl = (uint64_t)cmdtbl;
    cmdheader->ctba = (uint32_t)phys_cmdtbl;
    cmdheader->ctbau = (uint32_t)(phys_cmdtbl) >> 32;

    h2d_fis *fis = (h2d_fis*) &cmdtbl->cfis;

    fis->fis_type = 0x27;                   // H2D FIS
    fis->pm_port = 0x80;                    // Command
    fis->command = ATA_CMD_READ_DMA_EX;
    fis->lba0 = lba;
    fis->lba1 = lba >> 8;
    fis->lba2 = lba >> 16;
    fis->device = 0x40;                     // LBA mode
    fis->lba3 = lba >> 24;
    fis->lba4 = lba >> 32;
    fis->lba5 = lba >> 40;
    fis->countl = count & 0xFF;
    fis->counth = (count >> 8) & 0xFF;

    // Setup PRDT entry using hba_prdt_entry structure
    cmdtbl->prdt[0].dba = (uint32_t)buffer;
    cmdtbl->prdt[0].dbau = ((uint32_t)buffer) >> 32;

    cmdtbl->prdt[0].dbc = (count * 512) - 1; // 512 bytes per sector, minus one
    cmdtbl->prdt[0].rsvd = 0;
    cmdtbl->prdt[0].i = 1;                   // Interrupt on completion

    // Issue command
    port->ci = 1; // Use command slot 0
    while (port->ci & 1); // Wait for completion

    kheap_free((void *)cmdtbl,  sizeof(hba_cmd_table));
    return 0;
}


int ahci_write(uint64_t lba, uint32_t count, void *buffer) {
    if (!sata_disk.initialized) return -1;
    hba_mem *hba = (hba_mem*)sata_disk.abar;
    hba_port_t *port = &hba->ports[0]; // Using port 0

    // Initialize command header pointer from the port's command list base.
    volatile hba_cmd_header *cmdheader = (volatile hba_cmd_header *)(uintptr_t)port->clb;
    
    // Prepare command header for slot 0
    cmdheader[0].cfl = sizeof(h2d_fis) / 4;  // FIS length in DWORDS
    cmdheader[0].w = 1;                      // Write operation
    cmdheader[0].prdtl = 1;                  // One PRDT entry

    // Allocate and clear a command table for this slot
    hba_cmd_table *cmdtbl = (hba_cmd_table *) kmalloc_a(sizeof(hba_cmd_table), 1);
    memset(cmdtbl, 0, sizeof(hba_cmd_table));

    // Setup Host-to-Device FIS in the command table
    h2d_fis *fis = (h2d_fis *)&cmdtbl->cfis;
    fis->fis_type = 0x27;                  // H2D FIS
    fis->pm_port = 0x80;                   // Command (bit 7 set)
    fis->command = ATA_CMD_WRITE_DMA_EX;
    fis->lba0 = lba;
    fis->lba1 = lba >> 8;
    fis->lba2 = lba >> 16;
    fis->device = 0x40;                    // LBA mode
    fis->lba3 = lba >> 24;
    fis->lba4 = lba >> 32;
    fis->lba5 = lba >> 40;
    fis->countl = count & 0xFF;
    fis->counth = (count >> 8) & 0xFF;

    // Setup PRDT entry using hba_prdt_entry structure
    cmdtbl->prdt[0].dba = (uint32_t)buffer;
    cmdtbl->prdt[0].dbau = ((uint32_t)buffer) >> 32;
    cmdtbl->prdt[0].dbc = (count * 512) - 1; // (Transfer size in bytes) - 1
    cmdtbl->prdt[0].rsvd = 0;
    cmdtbl->prdt[0].i = 1; // Interrupt on completion

    // Link the command table to the command header (slot 0)
    uint64_t phys_cmdtbl = (uint64_t)cmdtbl;
    cmdheader[0].ctba = (uint32_t)phys_cmdtbl;
    cmdheader[0].ctbau = (uint32_t)(phys_cmdtbl) >> 32;

    // Issue command by setting the command issue bit for slot 0
    port->ci = 1; // Using command slot 0
    // Wait for command to complete (consider adding a timeout)
    while (port->ci & 1);

    return 0;
}


