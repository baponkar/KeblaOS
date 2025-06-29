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

#include "../timer/tsc.h"
#include  "../../memory/paging.h"

#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../memory/kmalloc.h"
#include "../../memory/kheap.h"
#include "../../memory/vmm.h"

#include "ahci.h"



// Checks if a port has a valid, active device attached.
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
                    return i;
                    break;
                case AHCI_DEV_SATAPI:
                    printf(" [AHCI] SATAPI drive found at port: %d \n", i);
                    // return i;
                    break;
                case AHCI_DEV_SEMB:
                    printf(" [AHCI] SEMB drive found at port: %d \n", i);
                    // return i;
                    break;
                case AHCI_DEV_PM:
                    printf(" [AHCI] PM drive found at port: %d \n", i);
                    // return i;
                    break;
			    default:
                    printf(" [AHCI] No drive found at port: %d \n", i);
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

	printf("[AHCI] Successfully Started CMD Engine\n");
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
		if (port->cmd & HBA_PxCMD_FR){
			continue;
		}
		if(port->cmd & HBA_PxCMD_CR){
			continue;
		}
		break;
	}

	printf("[AHCI] Successfully Stopped CMD Engine\n");
}

// AHCI driver to rebase a SATA port to use new memory locations for its command lis
void portRebase(HBA_MEM_T *abar, int port_no)
{
	HBA_PORT_T *port = (HBA_PORT_T *) &abar->ports[port_no];

	stopCMD(port);	// Stop command engine

	uint32_t AHCI_BASE = (uint32_t) kmalloc(0x1000); // although 0x400 i.e. 1KB memory need

	// Command list offset: 1K * port_no
	// Command list entry size = 32
	// Command list entry maximum count = 32
	// Command list maximum size = 32 * 32 = 1K per port
	port->clb = AHCI_BASE + (port_no << 10);
	port->clbu = 0;
	uint64_t vir_clb = phys_to_vir(port->clb); // convert phys to virt
	memset((void*) (uint64_t) (vir_clb), 0, 0x400);


	// FIS offset: 32K + 256 * port_no
	// FIS entry size = 256 bytes per port
	port->fb = AHCI_BASE + (32 << 10) + (port_no << 8);

	port->fbu = 0;
	uint64_t vir_fb = phys_to_vir(port->fb);
	memset((void*) (uint64_t) (vir_fb), 0, 0x100);
 
	// Command table offset: 40K + 8K * port_no
	// Command table size = 256 * 32 = 8K per port
	HBA_CMD_HEADER_T* cmd_header = (HBA_CMD_HEADER_T*) vir_clb;

	for (size_t i = 0; i < 32; i++)
	{
		cmd_header[i].prdtl = 8;

		// Calculate the physical address of the command table
		uint64_t phys_ctba = AHCI_BASE + (40 << 10) + (port_no << 13) + (i << 8);
		uint64_t virt_ctba = phys_to_vir(phys_ctba);  // Map to virtual

		// Set physical address in the command header (hardware uses physical)
		cmd_header[i].ctba = phys_ctba;
		cmd_header[i].ctbau = 0;

		// Clear the memory using the virtual address
		memset((void*)virt_ctba, 0, 0x100);
	}

    // Start command engine
	startCMD(port);	

	printf("[AHCI] Successfully port rebase implemented.\n");
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
		printf(" [AHCI] find free command list entry at %d\n", i);
	}
	printf(" [AHCI] Cannot find free command list entry\n");
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
	memset((void *)cmd_tbl, 0, sizeof(HBA_CMD_TBL_T) + (cmd_header->prdtl - 1) * sizeof(HBA_PRDT_ENTRY_T));
 
	// 8K bytes (16 sectors) per PRDT
    uint16_t i;
	for (i = 0; i < cmd_header->prdtl - 1; i++)
	{
		cmd_tbl->prdt_entry[i].dba = (uint32_t)((uint64_t)buf);
        // 8K bytes (this value should always be set to 1 less than the actual value)
		cmd_tbl->prdt_entry[i].dbc = 8 * 1024 - 1;	
		cmd_tbl->prdt_entry[i].i = 1;
        // 4K words
		buf += 4 * 1024;	// Move buffer pointer by 8K bytes (4K uint16_t elements)
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
		printf(" [AHCI] Port is hung\n");
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
			printf(" [AHCI] Read disk error\n");
			return false;
		}
	}

	// Check again
	if (port->is & HBA_PxIS_TFES)
	{
		printf(" [AHCI] Read disk error\n");
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


uint64_t get_total_sectors(uint16_t* identify_buf) {
    // Check if LBA48 is supported
    bool lba48_supported = identify_buf[83] & (1 << 10);

    if (lba48_supported) {
		// Word 100 = low word, Word 103 = high word
        uint64_t total_sectors =
            ((uint64_t)identify_buf[103] << 48) |
            ((uint64_t)identify_buf[102] << 32) |
            ((uint64_t)identify_buf[101] << 16) |
            (uint64_t)identify_buf[100];
        return total_sectors;
    } else {
        // Fallback to 28-bit
        uint32_t total_sectors =
            ((uint32_t)identify_buf[61] << 16) |
            (uint32_t)identify_buf[60];
        return total_sectors;
    }
}


void ahci_identify(HBA_PORT_T* port) {
	
    FIS_REG_H2D_T fis;
    memset(&fis, 0, sizeof(fis));
    fis.fis_type = FIS_TYPE_REG_H2D;
    fis.command = FIS_TYPE_ATA_CMD_IDENTIFY;	// 0xEC
    fis.device = 0;								// Master device
    fis.c = 1;									// Write command register

    uint16_t *identify_buf = (uint16_t *) kmalloc(512); // Allocate 512 bytes for IDENTIFY data
	if (identify_buf == NULL) {
		printf("[AHCI] Memory allocation for IDENTIFY buffer failed!\n");
		return;
	}

	if(runCommand(FIS_TYPE_ATA_CMD_IDENTIFY, 0, port, 0, 0, 1, identify_buf)){
        uint64_t sectors = get_total_sectors(identify_buf);
        uint64_t size_mb = (sectors * 512) / (1024 * 1024);

        printf("[AHCI] Disk Total Sectors: %d\n", sectors);
        printf("[AHCI] Disk Size: %d MB\n", size_mb);
    } else {
        printf("[AHCI] IDENTIFY command failed.\n");
    }
}




void test_ahci(HBA_MEM_T* abar)
{
	printf("[Info] Start Testing AHCI\n");

	if(!abar){
		printf("[Error] ABAR is Null\n");
	}

    HBA_PORT_T* port = &abar->ports[0];

	if(!port) {
		printf(" [-] AHCI: Port is NULL!\n");
		return;
	}

	probePort(abar);

	printf(" [-] AHCI Device Type: ");
	switch(checkType(port)){
		case AHCI_DEV_SATAPI:
			printf("AHCI_DEV_SATAPI\n");
			break;
		case AHCI_DEV_SEMB:
			printf("AHCI_DEV_SEMB\n");
			break;
		case AHCI_DEV_PM:
			printf("AHCI_DEV_PM\n");
			break;
		default:
			printf("AHCI_DEV_SATA\n");
			break;
	}


	// Create a buffer for the command list
    uint16_t* buf_1 = (uint16_t*)vir_to_phys((uint64_t)kheap_alloc(0x8000, ALLOCATE_DATA)); // 32 KB buffer (overkill but fine for test)
    if (buf_1 == NULL) {
        printf(" [AHCI] Buffer_1 Memory allocation failed!\n");
        return;
    }
    memset(buf_1, 0, 512); // Clear first sector (only 512 bytes needed)

    // Step 2: Write a string into the buffer
    const char* test_string = "Hello from KeblaOS!";
    memcpy((char*)buf_1, test_string, strlen(test_string)); // Copy into buffer

    // Step 3: Write the buffer to disk
    if (ahci_write(port, 0, 0, 1, buf_1)) {
        printf(" [AHCI] Write successful from buf_1 into disk.\n");
    } else {
        printf(" [AHCI] Write failed from buf_1 into disk!\n");
        return;
    }


	uint16_t* buf_2 = (uint16_t*)vir_to_phys((uint64_t)kheap_alloc(0x8000, ALLOCATE_DATA)); // 32 KB buffer (overkill but fine for test)
    if (buf_2 == NULL) {
        printf(" [AHCI] Buffer_2 Memory allocation failed!\n");
        return;
    }
    memset(buf_2, 0, 512);

    // Step 5: Read back from disk
    if (ahci_read(port, 0, 0, 1, buf_2)) {
        printf(" [AHCI] Data read from disk: %s.\n", (char*)buf_2);
    } else {
        printf(" [AHCI] Read failed from disk!\n");
    }

    kheap_free((void *)phys_to_vir((uint64_t) buf_1), 0x8000); 	// Free memory
	kheap_free((void *)phys_to_vir((uint64_t) buf_2), 0x8000);	// Free memory

	printf("[AHCI] test completed successfully.\n");
	return;
}
