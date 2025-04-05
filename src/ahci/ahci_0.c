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
    hba_mem_t *hba = (hba_mem_t*)(sata_disk.abar);
    
    // Enable AHCI in GHC
    hba->ghc |= HBA_GHC_AHCI_ENABLE;
    while (!(hba->ghc & HBA_GHC_AHCI_ENABLE)); // Wait for enable

    // Find active port
    uint32_t pi = hba->pi;
    for (int i = 0; i < 32; i++) {
        if (pi & (1 << i)) {
            hba_port_t *port = ( hba_port_t *)&hba->ports[i];
            uint32_t ssts = port->ssts;
            if ((ssts & 0xF) == 3) { // Device detected
                ahci_port_init(port);
                sata_disk.initialized = true;
                printf("[INFO] AHCI SATA Initialized.\n");
                return;
            }
        }
    }
    printf("AHCI: No active port found\n");
}


void ahci_port_init(hba_port_t *port) {
    // Stop command engine
    port->cmd &= ~HBA_PORT_CMD_ST;
    while (port->cmd & HBA_PORT_CMD_CR);

    // Allocate command list and FIS
    // Assuming aligned allocation functions exist
    void *cmd_list = (void *) kmalloc_aligned(1024, 1024); // 32 entries * 32 bytes
    void *fis = (void *) kmalloc_aligned(256, 256);

    // Set command list and FIS base
    port->clb = (uint32_t)cmd_list;
    port->clbu = (uint32_t)cmd_list >> 32;
    port->fb = (uint32_t)fis;
    port->fbu = (uint32_t)fis >> 32;

    // Enable FIS and start engine
    port->cmd |= HBA_PORT_CMD_FRE;
    port->cmd |= HBA_PORT_CMD_ST;
}



int ahci_read(uint64_t lba, uint32_t count, void *buffer) {
    if (!sata_disk.initialized) return -1;
    hba_mem_t *hba = (hba_mem_t *)sata_disk.abar;
    hba_port_t *port = &hba->ports[0]; // Assuming port 0

    // Prepare command header
    volatile hba_cmd_header_t *cmdheader = (volatile hba_cmd_header_t *)(uintptr_t)port->clb;;     // Access command list
    cmdheader->cfl = sizeof(h2d_fis_t) / 4;     // Size in DWORDS
    cmdheader->w = 0;                       // Read
    cmdheader->prdtl = 1;                   // 1 PRDT entry

    // Setup command table
    hba_cmd_table_t *cmdtbl = (hba_cmd_table_t *) kmalloc_aligned(sizeof(hba_cmd_table_t), 128);
    memset(cmdtbl, 0, sizeof(hba_cmd_table_t));

    // In ahci_read():
    // After allocating cmdtbl, add:
    uint64_t phys_cmdtbl = (uint64_t)cmdtbl;
    cmdheader->ctba = (uint32_t)phys_cmdtbl;
    cmdheader->ctbau = (uint32_t)(phys_cmdtbl) >> 32;

    h2d_fis_t *fis = (h2d_fis_t*) &cmdtbl->cfis;

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

    kheap_free((void *)cmdtbl,  sizeof(hba_cmd_table_t));
    
    return 0;
}


int ahci_write(uint64_t lba, uint32_t count, void *buffer) {
    if (!sata_disk.initialized) return -1;
    hba_mem_t *hba = (hba_mem_t *)sata_disk.abar;
    hba_port_t *port = &hba->ports[0]; // Using port 0

    // Initialize command header pointer from the port's command list base.
    hba_cmd_header_t *cmdheader = (hba_cmd_header_t *)(uintptr_t)port->clb;
    
    // Prepare command header for slot 0
    cmdheader[0].cfl = sizeof(h2d_fis_t) / 4;  // FIS length in DWORDS
    cmdheader[0].w = 1;                      // Write operation
    cmdheader[0].prdtl = 1;                  // One PRDT entry

    // Allocate and clear a command table for this slot
    hba_cmd_table_t *cmdtbl = (hba_cmd_table_t *) kmalloc_aligned(sizeof(hba_cmd_table_t), 128);
    memset(cmdtbl, 0, sizeof(hba_cmd_table_t));

    // Setup Host-to-Device FIS in the command table
    h2d_fis_t *fis = (h2d_fis_t *)&cmdtbl->cfis;
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


void test_ahci(ahci_controller_t sata_disk){
    if (sata_disk.abar != 0) {

        char *buffer = (char *) kmalloc_aligned(512, 512);
        memset(buffer, 0, 512); // Clearing buffer
        check_mem_alloc((void *)buffer, 512);

        if(ahci_read(0, 1, (void *)buffer) == 0) { 
            printf("Disk Read Successful at LBA 0!\n");
        
            // Check MBR signature (last 2 bytes of sector)
            if (buffer[510] == 0x55 && buffer[511] == 0xAA) {
                printf("Valid MBR Signature found!\n");
            } else {
                printf("No MBR found. Disk may be empty or unformatted.\n");
            }
        } else {
            printf("AHCI Read Failed!\n");
        }

        char *write_buffer = (char *) kmalloc_aligned(512, 512);
        memset(write_buffer, 'A', 512);  // Fill buffer with 'A'

        if(ahci_write(0, 1, write_buffer) == 0) {
            printf("AHCI Write Successful at LBA 0!\n");
        } else {
            printf("AHCI Write Failed at LBA 0!\n");
        }

        char *read_buffer = (char *) kmalloc_aligned(512, 512);
        memset(read_buffer, 0, 512);

        if (ahci_read(0, 1, read_buffer) == 0) {
            if (memcmp(write_buffer, read_buffer, 512) == 0) {
                printf("Write Verification Successful at LBA 0! Data matches. %x\n", *write_buffer);
            } else {
                printf("Write Verification Failed at LBA 0! Data does not match.\n");
            }
        } else {
            printf("Failed to read back written data.\n");
        }
    }
}