/*

AHCI  : Advance Host Controller Interface - is developed by Intel to facilitate handling SATA devices. 

ATA   : Advanced Technology Attachment
ATAPI : Advanced Technology Attachment Packet Interface (Serial) - Used for most modern optical drives.
ATAPI : Advanced Technology Attachment Packet Interface (Parallel) - Commonly used for optical drives.

PCI  : Peripheral Component Interconnect
PATA : Parallel Advanced Technology Attachment
SATA : Serial Advanced Technology Attachment
HBA  : Host Base Address
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

#include "../../../sys/timer/tsc.h"
#include  "../../../memory/paging.h"

#include "../../../lib/stdio.h"
#include "../../../lib/string.h"
#include "../../../memory/kmalloc.h"
#include "../../../memory/kheap.h"
#include "../../../memory/vmm.h"

#include "ahci.h"



// Checks if a port has a valid, active device
int checkType(HBA_PORT_T* port)
{
    if(port == NULL) {
        return AHCI_DEV_NULL;
    }
	uint32_t ssts = port->ssts;
 
	uint8_t ipm = (ssts >> 8) & 0x0F;
	uint8_t det = ssts & 0x0F;
 
	if (det != HBA_PORT_DET_PRESENT)	// Check drive status
		return AHCI_DEV_NULL;
	if (ipm != HBA_PORT_IPM_ACTIVE)
		return AHCI_DEV_NULL;
 
	switch (port->sig)
	{
        case SATA_SIG_ATAPI:
            return AHCI_DEV_SATAPI;
        case SATA_SIG_SEMB:
            return AHCI_DEV_SEMB;
        case SATA_SIG_PM:
            return AHCI_DEV_PM;
        default:
            return AHCI_DEV_SATA;
	}
}
 

// Find a port with a SATA drive
void probePort(HBA_MEM_T *abar)
{
	printf("[AHCI] Start Searching Ports\n");
	// Search disk in implemented ports
	uint32_t pi = abar->pi;
    for (size_t i = 0; i < 32; i++) 
	{
		if (pi & 1)
		{
            switch (checkType(&abar->ports[i])) 
            {
                case AHCI_DEV_SATA:
                    printf(" [AHCI] SATA drive found at port: %d\n", i);
                case AHCI_DEV_SATAPI:
                    printf(" [AHCI] SATAPI drive found at port: %d \n", i);
                case AHCI_DEV_SEMB:
                    printf(" [AHCI] SEMB drive found at port: %d \n", i);
                case AHCI_DEV_PM:
                    printf(" [AHCI] PM drive found at port: %d \n", i);
			    default:
                    printf(" [AHCI] No drive found at port: %d \n", i);
            }
		}
		pi >>= 1;
	}
} 


// Start command engine
void startCMD(HBA_PORT_T *port)
{
	// Wait until CR (bit15) is cleared
	while (port->cmd & HBA_PxCMD_CR);
 
	port->cmd |= HBA_PxCMD_FRE; // Set FRE (bit4)
	port->cmd |= HBA_PxCMD_ST;  // Set ST (bit0)

	// printf("[AHCI] Successfully Started CMD Engine\n");
}
 

// Stop command engine
void stopCMD(HBA_PORT_T *port)
{
    // Clear FRE (bit4)
	port->cmd &= ~HBA_PxCMD_FRE;

	// Clear ST (bit0)
	port->cmd &= ~HBA_PxCMD_ST;
 
 
	// Wait until FR (bit14), CR (bit15) are cleared
	while (true)
	{
		if (port->cmd & HBA_PxCMD_FR){
			continue;
		}
		if(port->cmd & HBA_PxCMD_CR){
			continue;
		}
		break;
	}

	// printf("[AHCI] Successfully Stopped CMD Engine\n");
}


// AHCI driver to rebase a SATA port to use new memory locations for its command list
void portRebase(HBA_MEM_T *abar, int port_no)
{
    HBA_PORT_T *port = &abar->ports[port_no];
    stopCMD(port);

    // Allocate a physically contiguous region for this port:
    // Make it large enough for CLB (1K), FB (1K), CTBA area (8K), plus margin.
    const size_t ALLOC_SIZE = 64 * 1024; // 64 KiB to be safe
    void *base_virt = (void *) kheap_alloc(ALLOC_SIZE, ALLOCATE_DATA);
    if (!base_virt) {
        printf("[AHCI] portRebase: kmalloc failed\n");
        return;
    }

    // Convert to physical base (what HBA will use)
    uintptr_t base_phys = vir_to_phys((uintptr_t)base_virt);
    if (base_phys == 0) {
        printf("[AHCI] portRebase: vir_to_phys returned 0\n");
        return;
    }

    // Layout within the allocated block (choose simple contiguous layout)
    // CLB: offset 0
    uintptr_t clb_phys = base_phys + 0x0;
    void *clb_virt = (void *)((uintptr_t)base_virt + 0x0); // virtual pointer for CPU
    memset(clb_virt, 0, 0x400); // 1 KiB

    // FB: offset 4K (use 4 KiB aligned area)
    uintptr_t fb_phys = base_phys + 0x1000;
    void *fb_virt = (void *)((uintptr_t)base_virt + 0x1000);
    memset(fb_virt, 0, 0x100); // 256 bytes (FIS receive area), zeroed

    // Command tables area: offset 8K (we'll allocate 8 KiB per port for command tables)
    uintptr_t ctba_base_phys = base_phys + 0x2000;
    void *ctba_base_virt = (void *)((uintptr_t)base_virt + 0x2000);
    // Zero the whole CTBA area (32 * 256 = 8192)
    memset(ctba_base_virt, 0, 0x2000);

    // Program registers (hardware uses physical addresses)
    port->clb  = (uint32_t)(clb_phys & 0xFFFFFFFF);
    port->clbu = (uint32_t)(clb_phys >> 32);

    port->fb   = (uint32_t)(fb_phys & 0xFFFFFFFF);
    port->fbu  = (uint32_t)(fb_phys >> 32);

    // Now initialize each command header to point into the CTBA area
    HBA_CMD_HEADER_T* cmd_header = (HBA_CMD_HEADER_T*) clb_virt; // CPU-side pointer into CLB area
    for (int i = 0; i < 32; ++i) {
        // Each command table sized 256 bytes (0x100) at sequential offsets
        uintptr_t phys_ctba = ctba_base_phys + (i * 0x100);
        cmd_header[i].ctba  = (uint32_t)(phys_ctba & 0xFFFFFFFF);
        cmd_header[i].ctbau = (uint32_t)(phys_ctba >> 32);
        cmd_header[i].prdtl = 8; // example default
        // Clear command table memory (use virtual)
        void* virt_ctba = (void*)((uintptr_t)ctba_base_virt + (i * 0x100));
        memset(virt_ctba, 0, 0x100);
    }

    // Start command engine after setting CLB/FB/CTBA
    startCMD(port);

    // printf("[AHCI] portRebase: done for port %d (phys base %x)\n", port_no, (unsigned long)base_phys);
}


// Find a free command list slot
int findCMDSlot(HBA_PORT_T* port, size_t cmd_slots)
{
	// If not set in SACT and CI, the slot is free
	uint32_t slots = port->sact | port->ci;
    for (uint32_t i = 0; i < cmd_slots; i++)
	{
		if (!(slots & 1))
			return i;
		slots >>= 1;
		printf(" [AHCI] find a free command list entry at %d\n", i);
	}
	printf(" [AHCI] Cannot find a free command list entry\n");
	return -1;
}


// Execute read or write command
// runCommand needs physical address of the buffer
bool runCommand(FIS_TYPE type, uint8_t write, HBA_PORT_T *port, uint32_t start_l, uint32_t start_h, uint32_t count, uintptr_t phys_buf){
    // Clear pending interrupt bits
    port->is = (uint32_t)-1;

    int spin = 0;
    int slot = findCMDSlot(port, 32);
    if (slot == -1) return false;

    // Convert CLB physical address (in port registers) to a virtual pointer for CPU use
    uintptr_t clb_phys = ((uint64_t)port->clb) | ((uint64_t)port->clbu << 32);
    HBA_CMD_HEADER_T* cmd_header_base = (HBA_CMD_HEADER_T*) (uintptr_t) phys_to_vir(clb_phys);
    if (!cmd_header_base) {
        printf(" [AHCI] runCommand: clb phys_to_vir failed\n");
        return false;
    }

    HBA_CMD_HEADER_T* cmd_header = (HBA_CMD_HEADER_T*) &cmd_header_base[slot];

    cmd_header->cfl = sizeof(FIS_REG_H2D_T) / sizeof(uint32_t);
    cmd_header->w   = write;
    cmd_header->prdtl = (uint16_t)(((count - 1) >> 4) + 1);

    // Compute CTBA physical (combine ctba + ctbau) and convert to virtual
    uint64_t ctba_phys = ((uint64_t)cmd_header->ctbau << 32) | (uint64_t)cmd_header->ctba;
    HBA_CMD_TBL_T* cmd_tbl = (HBA_CMD_TBL_T*)(uintptr_t) phys_to_vir(ctba_phys);
    if (!cmd_tbl) {
        printf(" [AHCI] runCommand: ctba phys_to_vir failed\n");
        return false;
    }

    // Clear command table area
    size_t cmdtbl_size = sizeof(HBA_CMD_TBL_T) + (cmd_header->prdtl - 1) * sizeof(HBA_PRDT_ENTRY_T);
    memset(cmd_tbl, 0, cmdtbl_size);

    // Fill PRDT entries: each 8 KiB (16 sectors)
    uint16_t i;
    uintptr_t cur_phys = phys_buf;
    uint32_t remaining = count;
    for (i = 0; i < cmd_header->prdtl - 1; i++) {
        cmd_tbl->prdt_entry[i].dba  = (uint32_t)(cur_phys & 0xFFFFFFFF);
        cmd_tbl->prdt_entry[i].dbau = (uint32_t)(cur_phys >> 32);
        cmd_tbl->prdt_entry[i].dbc  = 8 * 1024 - 1; // 8KB - 1
        cmd_tbl->prdt_entry[i].i    = 1;
        cur_phys += 8 * 1024;
        remaining -= 16;
    }

    // Last entry
    cmd_tbl->prdt_entry[i].dba  = (uint32_t)(cur_phys & 0xFFFFFFFF);
    cmd_tbl->prdt_entry[i].dbau = (uint32_t)(cur_phys >> 32);
    cmd_tbl->prdt_entry[i].dbc  = (remaining << 9) - 1; // remaining * 512 - 1
    cmd_tbl->prdt_entry[i].i    = 1;

    // Fill FIS
    FIS_REG_H2D_T* cmd_fis = (FIS_REG_H2D_T*)(&cmd_tbl->cfis);
    memset(cmd_fis, 0, sizeof(*cmd_fis));
    cmd_fis->fis_type = FIS_TYPE_REG_H2D;
    cmd_fis->c = 1;
    cmd_fis->command = type;

    // LBA fields
    cmd_fis->lba0 = (uint8_t) start_l;
    cmd_fis->lba1 = (uint8_t) (start_l >> 8);
    cmd_fis->lba2 = (uint8_t) (start_l >> 16);
    cmd_fis->lba3 = (uint8_t) (start_l >> 24);
    cmd_fis->lba4 = (uint8_t) start_h;
    cmd_fis->lba5 = (uint8_t) (start_h >> 8);

    cmd_fis->device = 1 << 6; // LBA mode
    cmd_fis->countl = (uint8_t)(count & 0xFF);
    cmd_fis->counth = (uint8_t)((count >> 8) & 0xFF);

    // Wait for port not busy
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000) spin++;
    if (spin == 1000000) {
        printf(" [AHCI] Port is hung\n");
        return false;
    }

    // Issue command
    port->ci = 1 << slot;

    // Wait for completion
    while (true) {
        if (!(port->ci & (1 << slot))) break;
        if (port->is & HBA_PxIS_TFES) {
            printf(" [AHCI] Disk error\n");
            return false;
        }
    }

    if (port->is & HBA_PxIS_TFES) {
        printf(" [AHCI] Disk error after completion\n");
        return false;
    }

    return true;
}















