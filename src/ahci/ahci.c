/*

*/

#include "ahci.h"
#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../driver/io/ports.h"
#include "../memory/kmalloc.h"



#define	SATA_SIG_ATA	0x00000101	// SATA drive
#define	SATA_SIG_ATAPI	0xEB140101	// SATAPI drive
#define	SATA_SIG_SEMB	0xC33C0101	// Enclosure management bridge
#define	SATA_SIG_PM	    0x96690101	// Port multiplier

#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

void probe_port(HBA_MEM_T *abar)
{
	// Search disk in implemented ports
	uint32_t pi = abar->pi;
	int i = 0;
	while (i<32)
	{
		if (pi & 1)
		{
			int dt = check_type(&abar->ports[i]);
			if (dt == AHCI_DEV_SATA)
			{
				printf("SATA drive found at port %d\n", i);
			}
			else if (dt == AHCI_DEV_SATAPI)
			{
				printf("SATAPI drive found at port %d\n", i);
			}
			else if (dt == AHCI_DEV_SEMB)
			{
				printf("SEMB drive found at port %d\n", i);
			}
			else if (dt == AHCI_DEV_PM)
			{
				printf("PM drive found at port %d\n", i);
			}
			else
			{
				printf("No drive found at port %d\n", i);
			}
		}

		pi >>= 1;
		i ++;
	}
}

// Check device type
static int check_type(HBA_PORT_T *port)
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


#define	AHCI_BASE	0x400000	// 4M

#define HBA_PxCMD_ST    0x0001
#define HBA_PxCMD_FRE   0x0010
#define HBA_PxCMD_FR    0x4000
#define HBA_PxCMD_CR    0x8000

void port_rebase(HBA_PORT_T *port, int portno)
{
	stop_cmd(port);	// Stop command engine

	// Command list offset: 1K*portno
	// Command list entry size = 32
	// Command list entry maxim count = 32
	// Command list maxim size = 32*32 = 1K per port
	port->clb = AHCI_BASE + (portno<<10);
	port->clbu = 0;
	memset((void*)(port->clb), 0, 1024);

	// FIS offset: 32K+256*portno
	// FIS entry size = 256 bytes per port
	port->fb = AHCI_BASE + (32<<10) + (portno<<8);
	port->fbu = 0;
	memset((void*)(port->fb), 0, 256);

	// Command table offset: 40K + 8K*portno
	// Command table size = 256*32 = 8K per port
	HBA_CMD_HEADER_T *cmdheader = (HBA_CMD_HEADER_T *)(port->clb);
	for (int i=0; i<32; i++)
	{
		cmdheader[i].prdtl = 8;	// 8 prdt entries per command table
					// 256 bytes per command table, 64+16+48+16*8
		// Command table offset: 40K + 8K*portno + cmdheader_index*256
		cmdheader[i].ctba = AHCI_BASE + (40<<10) + (portno<<13) + (i<<8);
		cmdheader[i].ctbau = 0;
		memset((void*)cmdheader[i].ctba, 0, 256);
	}

	start_cmd(port);	// Start command engine
}

// Start command engine
void start_cmd(HBA_PORT_T *port)
{
	// Wait until CR (bit15) is cleared
	while (port->cmd & HBA_PxCMD_CR)
		;

	// Set FRE (bit4) and ST (bit0)
	port->cmd |= HBA_PxCMD_FRE;
	port->cmd |= HBA_PxCMD_ST; 
}

// Stop command engine
void stop_cmd(HBA_PORT_T *port)
{
	// Clear ST (bit0)
	port->cmd &= ~HBA_PxCMD_ST;

	// Clear FRE (bit4)
	port->cmd &= ~HBA_PxCMD_FRE;

	// Wait until FR (bit14), CR (bit15) are cleared
	while(1)
	{
		if (port->cmd & HBA_PxCMD_FR)
			continue;
		if (port->cmd & HBA_PxCMD_CR)
			continue;
		break;
	}

}


void ahci_read(HBA_PORT_T *port, uint64_t startlba, uint64_t count, void *buf){
    // Check if the port is ready for a new command
    while (port->cmd & HBA_PxCMD_CR) {
        // Wait for the command register to be clear
    }

    // Set up the command table and issue the read command
    HBA_CMD_HEADER_T *cmd_header = (HBA_CMD_HEADER_T *)(port->clb);
    HBA_CMD_TBL_T *cmd_tbl = (HBA_CMD_TBL_T *)(cmd_header->ctba);

    // Set up the command table for a read operation
    cmd_tbl->cfis[0] = 0x27; // Read DMA command
    cmd_tbl->cfis[1] = startlba & 0xFF;
    cmd_tbl->cfis[2] = (startlba >> 8) & 0xFF;
    cmd_tbl->cfis[3] = (startlba >> 16) & 0xFF;
    cmd_tbl->cfis[4] = (startlba >> 24) & 0xFF;
    cmd_tbl->cfis[5] = (startlba >> 32) & 0xFF;
    cmd_tbl->cfis[6] = count & 0xFF;
    cmd_tbl->cfis[7] = (count >> 8) & 0xFF;

    // Set up the data buffer address
    cmd_tbl->prdt_entry[0].dba = (uint32_t)(uintptr_t)buf;
    cmd_tbl->prdt_entry[0].dbc = count * 512; // Assuming sector size of 512 bytes

    // Start the command
    port->cmd |= HBA_PxCMD_CR;

    // Wait for the command to complete
    while (port->cmd & HBA_PxCMD_CR) {
        // Wait for the command register to be clear
    }
}


void ahci_write(HBA_PORT_T *port, uint64_t startlba, uint64_t count, void *buf){
    // Check if the port is ready for a new command
    while (port->cmd & HBA_PxCMD_CR) {
        // Wait for the command register to be clear
    }

    // Set up the command table and issue the write command
    HBA_CMD_HEADER_T *cmd_header = (HBA_CMD_HEADER_T *)(port->clb);
    HBA_CMD_TBL_T *cmd_tbl = (HBA_CMD_TBL_T *)(cmd_header->ctba);

    // Set up the command table for a write operation
    cmd_tbl->cfis[0] = 0x35; // Write DMA command
    cmd_tbl->cfis[1] = startlba & 0xFF;
    cmd_tbl->cfis[2] = (startlba >> 8) & 0xFF;
    cmd_tbl->cfis[3] = (startlba >> 16) & 0xFF;
    cmd_tbl->cfis[4] = (startlba >> 24) & 0xFF;
    cmd_tbl->cfis[5] = (startlba >> 32) & 0xFF;
    cmd_tbl->cfis[6] = count & 0xFF;
    cmd_tbl->cfis[7] = (count >> 8) & 0xFF;

    // Set up the data buffer address
    cmd_tbl->prdt_entry[0].dba = (uint32_t)(uintptr_t)buf;
    cmd_tbl->prdt_entry[0].dbc = count * 512; // Assuming sector size of 512 bytes

    // Start the command
    port->cmd |= HBA_PxCMD_CR;

    // Wait for the command to complete
    while (port->cmd & HBA_PxCMD_CR) {
        // Wait for the command register to be clear
    }
}



extern ahci_controller_t sata_disk;

void init_ahci(ahci_controller_t *sata_disk) {


    printf("[INFO] AHCI Controller Initialized!\n");
}