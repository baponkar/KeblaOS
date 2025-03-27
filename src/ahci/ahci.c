
/*
AHCI (Advance Host Controller Interface)
AHCI (Advance Host Controller Interface) is developed by Intel to facilitate handling SATA devices. 

References:
    https://wiki.osdev.org/AHCI
*/

#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../driver/io/ports.h"
#include "../memory/kmalloc.h"
#include "../memory/kheap.h"
#include "../disk/disk.h"

#include "ahci.h"





#define HBA_GHC_AHCI_ENABLE (1 << 31)
#define HBA_PORT_CMD_ST     (1 << 0)
#define HBA_PORT_CMD_FRE    (1 << 4)
#define HBA_PORT_CMD_CR     (1 << 15)

#define ATA_CMD_READ_DMA_EX 0x25
#define ATA_CMD_WRITE_DMA_EX 0x35


ahci_controller_t ahci_ctrl = {0};


void ahci_init() {
    hba_mem *hba = (hba_mem*)(ahci_ctrl.abar);
    
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
                ahci_ctrl.initialized = true;
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
    void *cmd_list = (void *)kmalloc_a(1024, 1); // 32 entries * 32 bytes
    void *fis = (void *) kmalloc_a(256, 1);

    // Set command list and FIS base
    port->clb = (uint32_t)(uintptr_t)cmd_list;
    port->clbu = (uint32_t)((uintptr_t)cmd_list >> 32);
    port->fb = (uint32_t)(uintptr_t)fis;
    port->fbu = (uint32_t)((uintptr_t)fis >> 32);

    // Enable FIS and start engine
    port->cmd |= HBA_PORT_CMD_FRE;
    port->cmd |= HBA_PORT_CMD_ST;
}



int ahci_read(uint64_t lba, uint32_t count, void *buffer) {
    if (!ahci_ctrl.initialized) return -1;
    hba_mem *hba = (hba_mem*)ahci_ctrl.abar;
    struct hba_port *port = &hba->ports[0]; // Assuming port 0

    // Prepare command header
    volatile hba_cmd_header *cmdheader;// Access command list
    cmdheader->cfl = sizeof(h2d_fis)/4;  // Size in DWORDS
    cmdheader->w = 0;                   // Read
    cmdheader->prdtl = 1;               // 1 PRDT entry

    // Setup command table
    hba_cmd_table *cmdtbl = (hba_cmd_table *)kheap_alloc(sizeof(hba_cmd_table));
    memset(cmdtbl, 0, sizeof(hba_cmd_table));
    h2d_fis *fis = (h2d_fis*)&cmdtbl->cfis;
    fis->fis_type = 0x27;               // H2D FIS
    fis->pm_port = 0x80;                // Command
    fis->command = ATA_CMD_READ_DMA_EX;
    fis->lba0 = lba;
    fis->lba1 = lba >> 8;
    fis->lba2 = lba >> 16;
    fis->device = 0x40;                 // LBA mode
    fis->lba3 = lba >> 24;
    fis->lba4 = lba >> 32;
    fis->lba5 = lba >> 40;
    fis->countl = count & 0xFF;
    fis->counth = (count >> 8) & 0xFF;

    // Setup PRDT
    cmdtbl->prdt[0] = (uint32_t)(uintptr_t)buffer; // Data buffer (low)
    cmdtbl->prdt[1] = (uint32_t)((uintptr_t)buffer >> 32); // High
    cmdtbl->prdt[2] = 0; // Reserved
    cmdtbl->prdt[3] = (count * 512) | (1 << 31); // Byte count and interrupt

    // Issue command
    port->ci = 1; // Use command slot 0
    while (port->ci & 1); // Wait for completion

    kheap_free((void *)cmdtbl,  sizeof(hba_cmd_table));
    return 0;
}

int ahci_write(uint64_t lba, uint32_t count, void *buffer){
	if (!ahci_ctrl.initialized) return -1;
	hba_mem *hba = (hba_mem*)ahci_ctrl.abar;
	struct hba_port *port = &hba->ports[0]; // Assuming port 0

	// Prepare command header
	volatile hba_cmd_header *cmdheader; // Access command list
	cmdheader->cfl = (uint8_t) sizeof(h2d_fis)/4;  // Size in DWORDS
	cmdheader->w = 1;                   // Write
	cmdheader->prdtl = 1;               // 1 PRDT entry

	// Setup command table
	hba_cmd_table *cmdtbl = (hba_cmd_table *) kmalloc_a(sizeof(hba_cmd_table), 1);
	memset(cmdtbl, 0, sizeof(hba_cmd_table));
	h2d_fis *fis = (h2d_fis*)&cmdtbl->cfis;
	fis->fis_type = 0x27;               // H2D FIS
	fis->pm_port = 0x80;                // Command
	fis->command = ATA_CMD_WRITE_DMA_EX;
	fis->lba0 = lba;
	fis->lba1 = lba >> 8;
	fis->lba2 = lba >> 16;
	fis->device = 0x40;                 // LBA mode
	fis->lba3 = lba >> 24;
	fis->lba4 = lba >> 32;
	fis->lba5 = lba >> 40;
	fis->countl = count & 0xFF;
	fis->counth = (count >> 8) & 0xFF;

	// Setup PRDT
	cmdtbl->prdt[0] = (uint32_t)(uintptr_t)buffer; // Data buffer (low)
	cmdtbl->prdt[1] = (uint32_t)((uintptr_t)buffer >> 32); // High
	cmdtbl->prdt[2] = 0; // Reserved
	cmdtbl->prdt[3] = (count * 512) | (1 << 31); // Byte count and interrupt

	// Issue command
	port->ci = 1; // Use command slot 0
	while (port->ci & 1); // Wait for completion
}

