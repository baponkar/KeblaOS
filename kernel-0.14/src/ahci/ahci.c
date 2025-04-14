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
#include "../memory/kmalloc.h"
#include "../memory/kheap.h"
#include "../memory/vmm.h"

#include "ahci.h"

extern ahci_controller_t sata_disk; // This detect by pci scan



// Check device type
static int checkType(HBA_PORT_T* port)
{
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
 
int probePort(HBA_MEM_T *abar)
{
	printf("[Info] Start Searching Ports\n");
	// Search disk in implemented ports
	uint32_t pi = abar->pi;
    for (size_t i = 0; i < 32; i++) 
	{
		if (pi & 1)
		{
            switch (checkType(&abar->ports[i])) 
            {
                case AHCI_DEV_SATA:
                    printf("[-] SATA drive found at port: %d\n", i);
                    return i;
                    break;
                case AHCI_DEV_SATAPI:
                    printf("[-] SATAPI drive found at port: %d \n", i);
                    // return i;
                    break;
                case AHCI_DEV_SEMB:
                    printf("[-] SEMB drive found at port: %d \n", i);
                    // return i;
                    break;
                case AHCI_DEV_PM:
                    printf("[-]] PM drive found at port: %d \n", i);
                    // return i;
                    break;
			    default:
                    printf("[-] No drive found at port: %d \n", i);
            }
		}
		pi >>= 1;
	}
    return -1;
} 

// Start command engine
void startCMD(HBA_PORT_T *port)
{
	// Wait until CR (bit15) is cleared
	while (port->cmd & HBA_PxCMD_CR);
 
	// Set FRE (bit4) and ST (bit0)
	port->cmd |= HBA_PxCMD_FRE;
	port->cmd |= HBA_PxCMD_ST; 
}
 
// Stop command engine
void stopCMD(HBA_PORT_T *port)
{
	// Clear ST (bit0)
	port->cmd &= ~HBA_PxCMD_ST;
 
	// Clear FRE (bit4)
	port->cmd &= ~HBA_PxCMD_FRE;
 
	// Wait until FR (bit14), CR (bit15) are cleared
	while (true)
	{
		if (port->cmd & HBA_PxCMD_FR)
			continue;
		if (port->cmd & HBA_PxCMD_CR)
			continue;
		break;
	}
}

void portRebase(HBA_PORT_T *port, int port_no)
{
	stopCMD(port);	// Stop command engine
 
	// Command list offset: 1K * port_no
	// Command list entry size = 32
	// Command list entry maximum count = 32
	// Command list maximum size = 32 * 32 = 1K per port
	port->clb = AHCI_BASE + (port_no << 10);
	port->clbu = 0;
	memset((void*) (uint64_t) (port->clb), 0, 0x400);
 
	// FIS offset: 32K + 256 * port_no
	// FIS entry size = 256 bytes per port
	port->fb = AHCI_BASE + (32 << 10) + (port_no << 8);
	port->fbu = 0;
	memset((void*) (uint64_t) (port->fb), 0, 0x100);
 
	// Command table offset: 40K + 8K * port_no
	// Command table size = 256 * 32 = 8K per port
	HBA_CMD_HEADER_T* cmd_header = (HBA_CMD_HEADER_T*) (uint64_t) (port->clb);
	for (size_t i = 0; i < 32; i++)
	{
        // 8 prdt entries per command table
        // 256 bytes per command table, 64+16+48+16*8
		cmd_header[i].prdtl = 8;	
                                  
		// Command table offset: 40K + 8K*port_no + cmd_header_index*256
		cmd_header[i].ctba = AHCI_BASE + (40 << 10) + (port_no << 13) + (i << 8);
		cmd_header[i].ctbau = 0;
		memset((void*) (uint64_t) cmd_header[i].ctba, 0, 0x100);
	}
    // Start command engine
	startCMD(port);	
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
	}
	printf("[-] Cannot find free command list entry\n");
	return -1;
}



static bool runCommand(FIS_TYPE type, uint8_t write, HBA_PORT_T *port, uint32_t start_l, uint32_t start_h, uint32_t count, uint16_t* buf)
{
    // Clear pending interrupt bits
	port->is = (uint32_t) -1;		
    // Spin lock timeout counter
	int spin = 0; 
	int slot = findCMDSlot(port, 32);

	if (slot == -1)
		return false;
 
	HBA_CMD_HEADER_T* cmd_header = (HBA_CMD_HEADER_T*) (uint64_t) port->clb;
	cmd_header += slot;
    // Command FIS size
	cmd_header->cfl = sizeof(FIS_REG_H2D_T) / sizeof(uint32_t);	
	// Read or write from device
    cmd_header->w = write;
    // PRDT entries count
	cmd_header->prdtl = (uint16_t) ((count - 1) >> 4) + 1;	
 
	HBA_CMD_TBL_T* cmd_tbl = (HBA_CMD_TBL_T*) (uint64_t) (cmd_header->ctba);
	memset(cmd_tbl, 0, sizeof(HBA_CMD_TBL_T) + (cmd_header->prdtl - 1) * sizeof(HBA_PRDT_ENTRY_T));
 
	// 8K bytes (16 sectors) per PRDT
    uint16_t i;
	for (i = 0; i < cmd_header->prdtl - 1; i++)
	{
		cmd_tbl->prdt_entry[i].dba = (uint32_t) buf;
        // 8K bytes (this value should always be set to 1 less than the actual value)
		cmd_tbl->prdt_entry[i].dbc = 8 * 1024 - 1;	
		cmd_tbl->prdt_entry[i].i = 1;
        // 4K words
		buf += 4 * 1024;	
        // 16 sectors
		count -= 16;	
	}
	// Last entry
	cmd_tbl->prdt_entry[i].dba = (uint32_t) buf;

    // 512 bytes per sector
	cmd_tbl->prdt_entry[i].dbc = (count << 9) - 1;	
	cmd_tbl->prdt_entry[i].i = 1;
 
	// Setup command
	FIS_REG_H2D_T* cmd_fis = (FIS_REG_H2D_T*) (&cmd_tbl->cfis);
	cmd_fis->fis_type = FIS_TYPE_REG_H2D;

    // Command
	cmd_fis->c = 1;	
	cmd_fis->command = type;

 	// LBA mode
	cmd_fis->lba0 = (uint8_t) start_l;
	cmd_fis->lba1 = (uint8_t) (start_l >> 8);
	cmd_fis->lba2 = (uint8_t) (start_l >> 16);
	cmd_fis->device = 1 << 6;
 
	cmd_fis->lba3 = (uint8_t) (start_l >> 24);
	cmd_fis->lba4 = (uint8_t) start_h;
	cmd_fis->lba5 = (uint8_t) (start_h >> 8);
 
	cmd_fis->countl = count & 0xFF;
	cmd_fis->counth = (count >> 8) & 0xFF;
 
	// The below loop waits until the port is no longer busy before issuing a new command
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{
		spin++;
	}
    
	if (spin == 1000000)
	{
		printf("[-] Port is hung\n");
		return false;
	}

    // Issue command
	port->ci = 1 << slot;	
 
	// Wait for completion
	while (true)
	{
		// In some longer duration reads, it may be helpful to spin on the DPS bit 
		// in the PxIS port field as well (1 << 5)
		if (!(port->ci & (1 << slot)))
			break;

        // Task file error
		if (port->is & HBA_PxIS_TFES)	
		{
			printf("[-] Read disk error\n");
			return false;
		}
	}
 
	// Check again
	if (port->is & HBA_PxIS_TFES)
	{
		printf("[-]] Read disk error\n");
		return false;
	}
 
	return true;
}

inline bool ahci_read(HBA_PORT_T* port, uint32_t start_l, uint32_t start_h, uint32_t count, uint16_t* buf) {
    return runCommand(ATA_CMD_READ_DMA_EX, 0, port, start_l, start_h, count, buf);
}

inline bool ahci_write(HBA_PORT_T* port, uint32_t start_l, uint32_t start_h, uint32_t count, uint16_t* buf) {
    return runCommand(ATA_CMD_WRITE_DMA_EX, 1, port, start_l, start_h, count, buf);
}


void test_ahci(ahci_controller_t controller)
{
	printf("[Info] Start Testing AHCI\n");
    HBA_MEM_T* abar = (HBA_MEM_T*) controller.abar;
    HBA_PORT_T* port = &abar->ports[0];

    uint16_t* buf_1 = (uint16_t*)(uintptr_t)vir_to_phys((uint64_t)kheap_alloc(0x8000)); // 32 KB buffer (overkill but fine for test)
    if (buf_1 == NULL) {
        printf("[-] Buffer_1 Memory allocation failed!\n");
        return;
    }
    memset(buf_1, 0, 512); // Clear first sector (only 512 bytes needed)

    // Step 2: Write a string into the buffer
    const char* test_string = "Hello from KeblaOS!";
    memcpy((char*)buf_1, test_string, strlen(test_string)); // Copy into buffer

    // Step 3: Write the buffer to disk
    if (ahci_write(port, 0, 0, 1, buf_1)) {
        printf("[-] Write successful from buf_1 into disk.\n");
    } else {
        printf("[-] Write failed from buf_1 into disk!\n");
        return;
    }


	uint16_t* buf_2 = (uint16_t*)(uintptr_t)vir_to_phys((uint64_t)kheap_alloc(0x8000)); // 32 KB buffer (overkill but fine for test)
    if (buf_2 == NULL) {
        printf("[-] Buffer_2 Memory allocation failed!\n");
        return;
    }
    memset(buf_2, 0, 512);
    // Step 5: Read back from disk
    if (ahci_read(port, 0, 0, 1, buf_2)) {
        printf("[-] Data read from disk: %s.\n", (char*)buf_2);
    } else {
        printf("[-] Read failed from disk!\n");
    }

    kheap_free((void *)phys_to_vir((uint64_t) buf_1), 0x8000); // Free memory
	kheap_free((void *)phys_to_vir((uint64_t) buf_2), 0x8000);
}
