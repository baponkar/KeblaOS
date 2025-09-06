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

#include "../../../sys/timer/tsc.h"
#include  "../../../memory/paging.h"

#include "../../../lib/stdio.h"
#include "../../../lib/string.h"
#include "../../../memory/kmalloc.h"
#include "../../../memory/kheap.h"
#include "../../../memory/vmm.h"

#include "ahci.h"


extern pci_device_t *mass_storage_controllers;  // Array to store detected mass storage devices
extern size_t mass_storage_count;               // Counter for mass storage devices

AHCI_Context *ahci_disks;                       // Store Different AHCI Disks contexts
int ahci_disks_count = 0;

bool is_ahci_mass_storage(pci_device_t *device){

    if (device->class_code == 0x01 &&       // Mass Storage
        device->subclass_code == 0x06 &&    // SATA
        device->prog_if == 0x01) {          // AHCI
        // This is an AHCI SATA Controller
        return true;
    }
    return false;
}


HBA_MEM_T* pci_dev_to_abar(pci_device_t* dev){

    if(dev == NULL){
        return NULL;
    }

    uintptr_t abar_phys = dev->base_address_registers[5] & ~0xF;    // BAR5 is usually used for AHCI
    HBA_MEM_T *abar = (HBA_MEM_T *) phys_to_vir(abar_phys);         // Map to virtual address

    return abar;
}


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
int probePort(HBA_MEM_T *abar)
{
	// printf("[AHCI] Start Searching Ports\n");
	// Search disk in implemented ports
	uint32_t pi = abar->pi;
    for (size_t i = 0; i < 32; i++) 
	{
		if (pi & 1)
		{
            switch (checkType(&abar->ports[i])) 
            {
                case AHCI_DEV_SATA:
                    // printf(" [AHCI] SATA drive found at port: %d\n", i);
                    return i;
                    break;
                case AHCI_DEV_SATAPI:
                    // printf(" [AHCI] SATAPI drive found at port: %d \n", i);
                    // return i;
                    break;
                case AHCI_DEV_SEMB:
                    // printf(" [AHCI] SEMB drive found at port: %d \n", i);
                    // return i;
                    break;
                case AHCI_DEV_PM:
                    // printf(" [AHCI] PM drive found at port: %d \n", i);
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

	// printf("[AHCI] Successfully Started CMD Engine\n");
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

	// printf("[AHCI] Successfully Stopped CMD Engine\n");
}


// AHCI driver to rebase a SATA port to use new memory locations for its command lis
void portRebase(HBA_MEM_T *abar, int port_no)
{
    HBA_PORT_T *port = &abar->ports[port_no];
    stopCMD(port);

    // Allocate a physically contiguous region for this port:
    // Make it large enough for CLB (1K), FB (1K), CTBA area (8K), plus margin.
    const size_t ALLOC_SIZE = 64 * 1024; // 64 KiB to be safe
    void *base_virt = kheap_alloc(ALLOC_SIZE, ALLOCATE_DATA);
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
		printf(" [AHCI] find free command list entry at %d\n", i);
	}
	printf(" [AHCI] Cannot find free command list entry\n");
	return -1;
}

// Execute read or write command
// runCommand needs physical address of the buffer
bool runCommand(FIS_TYPE type, uint8_t write,
                       HBA_PORT_T *port,
                       uint32_t start_l, uint32_t start_h,
                       uint32_t count, uintptr_t phys_buf)
{
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

    HBA_CMD_HEADER_T* cmd_header = &cmd_header_base[slot];

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




// LBA=start_l+((uint64_t)start_h<<32)
// start_l=LBA&0xFFFFFFFF
// start_h=(LBA>>32)&0xFFFFFFFF
// The below function is taking physical address of buffer 
inline bool ahci_read(HBA_PORT_T* port, uint64_t lba, uint32_t count, uintptr_t phys_buf) {
    uint64_t total_sectors = get_total_sectors(port);
    if(total_sectors > 0 && lba + count >= total_sectors){
        printf("No Space in AHCI Disk\n");
        return false;
    }
    
	uint32_t start_l = (uint32_t)(lba & 0xFFFFFFFF);
	uint32_t start_h = (uint32_t)((lba >> 32) & 0xFFFFFFFF);
    return runCommand(ATA_CMD_READ_DMA_EX, 0, port, start_l, start_h, count, phys_buf);
}


// LBA=start_l+((uint64_t)start_h<<32)
// start_l=LBA&0xFFFFFFFF
// start_h=(LBA>>32)&0xFFFFFFFF
// The below function is taking physical address of buffer 
inline bool ahci_write(HBA_PORT_T* port, uint64_t lba, uint32_t count, uintptr_t phys_buf) {
    uint64_t total_sectors = get_total_sectors(port);
    if(total_sectors > 0 && lba + count >= total_sectors){
        printf("No Space in AHCI Disk\n");
        return false;
    }

	uint32_t start_l = (uint32_t)(lba & 0xFFFFFFFF);
	uint32_t start_h = (uint32_t)((lba >> 32) & 0xFFFFFFFF);
    return runCommand(ATA_CMD_WRITE_DMA_EX, 1, port, start_l, start_h, count, phys_buf);
}


uint64_t get_total_sectors(HBA_PORT_T* port) {
    void *identify_buf = (void *) kheap_alloc(512, ALLOCATE_DATA); 								// Allocate 512 bytes for IDENTIFY data
    if (identify_buf == NULL) {
        printf(" [AHCI] Memory allocation for IDENTIFY buffer failed!\n");
        return 0;
    }
    memset(identify_buf, 0, 512); // Clear the buffer
    uintptr_t phys_identify_buf = (uintptr_t) vir_to_phys((uint64_t) identify_buf); // Convert to physical address

    // Send IDENTIFY command
    if(runCommand(ATA_CMD_IDENTIFY, 0, port, 0, 0, 1, phys_identify_buf)){

        uint16_t* id_buf_16 = (uint16_t*) identify_buf;

        // Check if LBA48 is supported
        bool lba48_supported = id_buf_16[83] & (1 << 10);

        if (lba48_supported) {
            // Word 100 = low word, Word 103 = high word
            uint64_t total_sectors =
                ((uint64_t)id_buf_16[103] << 48) |
                ((uint64_t)id_buf_16[102] << 32) |
                ((uint64_t)id_buf_16[101] << 16) |
                (uint64_t)id_buf_16[100];
            return total_sectors;
        } else {
            // Fallback to 28-bit
            uint32_t total_sectors =
                ((uint32_t)id_buf_16[61] << 16) |
                (uint32_t)id_buf_16[60];
            return total_sectors;
        }
    } else {
        printf(" [AHCI] IDENTIFY command failed.\n");
        return 0;
    }

    kheap_free(identify_buf, 512); // Free the allocated buffer

    return 0;
}


uint32_t get_bytes_per_sector(HBA_PORT_T* port) {
    void *identify_buf = (void *) kheap_alloc(512, ALLOCATE_DATA);
    if (identify_buf == NULL) {
        printf(" [AHCI] Memory allocation for IDENTIFY buffer failed!\n");
        return 512; // default
    }
    memset(identify_buf, 0, 512);
    uintptr_t phys_identify_buf = (uintptr_t) vir_to_phys((uint64_t) identify_buf);

    // Send IDENTIFY command
    if (runCommand(ATA_CMD_IDENTIFY, 0, port, 0, 0, 1, phys_identify_buf)) {
        uint16_t* id_buf_16 = (uint16_t*) identify_buf;

        // Word 106, bit 14 = "words per logical sector valid"
        bool logical_sector_valid = id_buf_16[106] & (1 << 14);

        if (logical_sector_valid) {
            uint32_t bytes_per_sector =
                ((uint32_t)id_buf_16[118] << 16) |
                (uint32_t)id_buf_16[117];
            if (bytes_per_sector != 0) {
                kheap_free(identify_buf, 512);
                return bytes_per_sector;
            }
        }

        // Default sector size = 512 bytes
        kheap_free(identify_buf, 512);
        return 512;

    } else {
        printf(" [AHCI] IDENTIFY command failed.\n");
        kheap_free(identify_buf, 512);
        return 512;
    }
}


HBA_PORT_T* abar_to_port(HBA_MEM_T* abar){
    if(abar == NULL){
        return NULL;
    }

    int p = probePort(abar);
    if (p < 0) {
        printf(" [-] AHCI: No SATA drive found!\n");
        return NULL;
    }

    return (HBA_PORT_T*) &abar->ports[p];	// Select the port
}





void ahci_identify(HBA_PORT_T* port) {

    uint64_t sectors = get_total_sectors(port);
    uint64_t size_mb = (sectors * 512) / (1024 * 1024);

    printf(" [AHCI] Disk Total Sectors: %d\n", sectors);
    printf(" [AHCI] Disk Size: %d MB\n", size_mb);

    uint16_t *buf = (uint16_t*)kheap_alloc(512, ALLOCATE_DATA);
    if (!buf) return;

    if (!runCommand(FIS_TYPE_REG_H2D, 0, port, 0, 0, 0, (uintptr_t)buf)) {
        printf("IDENTIFY DEVICE failed\n");
        kheap_free(buf, sizeof(buf));
        return;
    }

    char serial[21] = {0};
    char firmware[9] = {0};
    char model[41] = {0};

    for (int i = 0; i < 10; i++) {
        serial[i*2]   = (buf[10+i] >> 8) & 0xFF;
        serial[i*2+1] = buf[10+i] & 0xFF;
    }

    for (int i = 0; i < 4; i++) {
        firmware[i*2]   = (buf[23+i] >> 8) & 0xFF;
        firmware[i*2+1] = buf[23+i] & 0xFF;
    }

    for (int i = 0; i < 20; i++) {
        model[i*2]   = (buf[27+i] >> 8) & 0xFF;
        model[i*2+1] = buf[27+i] & 0xFF;
    }

    printf("AHCI Disk: Model: %s, SN: %s, FW: %s\n", model, serial, firmware);

    kheap_free(buf, sizeof(buf));
}



bool init_ahci(int ahci_disk_no){

    if(!mass_storage_controllers){
        printf("mass_storage_controllers is NULL\n");
        return false;
    }

    if(!mass_storage_count){
        printf("mass_storage_count is 0\n");
        return false;
    }

    if(ahci_disk_no > mass_storage_count){
        printf("invalid ahci disk no.\n");
    }

    pci_device_t device = mass_storage_controllers[ahci_disk_no];
    
    HBA_MEM_T* abar = pci_dev_to_abar(&device);
    if(!abar){
        printf("[AHCI] ABAR id NULL\n");
    }

    // Find a port with a SATA drive
    int p = probePort(abar);
    if (p < 0) {
        printf(" [-] AHCI: No SATA drive found!\n");
        return false;
    }
    portRebase(abar, p);	                            // Rebase the port
    HBA_PORT_T* port = (HBA_PORT_T*) &abar->ports[p];	// Select the port

    if(!port) {
        printf(" [-] AHCI: Port is NULL!\n");
        return false;
    }

    // If ahci_disks memory allocation not happened
    if(!ahci_disks){
        ahci_disks = (AHCI_Context *) kheap_alloc(sizeof(AHCI_Context)*mass_storage_count, ALLOCATE_DATA);
        memset(ahci_disks, 0, sizeof(AHCI_Context)*mass_storage_count);
    }

    if(!ahci_disks){
        printf("AHCI_CONTEXT alloc. failed!\n");
        return false;
    }

    ahci_disks[ahci_disks_count].abar = abar;
    ahci_disks[ahci_disks_count].port = port;

    ahci_disks_count++;

    return true;
}


void test_ahci(int ahci_disk_no)
{
	printf("[AHCI] Start Testing AHCI\n");

    if(!init_ahci(ahci_disk_no)){
        printf("AHCI initialization is failed!\n");
    }else{
        printf("Successfully AHCI Disks initialized\n");
    }

    AHCI_Context disk = ahci_disks[ahci_disk_no];

    HBA_MEM_T *abar = disk.abar;
    if(!abar){
        printf("ABAR is NULL\n");
    }

    HBA_PORT_T *port = disk.port;
    if(!port){
        printf("PORT is NULL\n");
    }

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

	// Get total sectors and size
	ahci_identify(port);

	// Create a buffer for the command list
    void *buf_1_vir = (void *)kheap_alloc(0x8000, ALLOCATE_DATA);       // 32 KB buffer (overkill but fine for test)
    if (buf_1_vir == NULL) {
        printf(" [AHCI] Buffer_1 Memory allocation failed!\n");
        return;
    }
    memset(buf_1_vir, 0, 512); // Clear first sector (only 512 bytes needed)
	uintptr_t buf_1_phys = (uintptr_t)vir_to_phys((uint64_t)buf_1_vir); // Convert to physical address

    // Step 2: Write a string into the buffer
    const char* test_string = "Hello from KeblaOS!\0";
    memcpy((char*)buf_1_vir, test_string, strlen(test_string)); 	// Copy into buffer

    // Step 3: Write the buffer to disk
    if (ahci_write(port, 0, 4, buf_1_phys)) {
        printf(" [AHCI] Write successful from buf_1 into disk.\n");
    } else {
        printf(" [AHCI] Write failed from buf_1 into disk!\n");
        return;
    }


	void * buf_2_vir = (void *)kheap_alloc(0x8000, ALLOCATE_DATA); // 32 KB buffer (overkill but fine for test)
	if (buf_2_vir == NULL) {
		printf(" [AHCI] Buffer_2 Memory allocation failed!\n");
		return;
	}
	memset(buf_2_vir, 0, 512); // Clear first sector (only 512 bytes needed)
	// Convert virtual address to physical for AHCI read
	uintptr_t buf_2_phys = (uintptr_t)vir_to_phys((uint64_t)buf_2_vir); // Convert to physical address

    // Step 5: Read back from disk
    if (ahci_read(port, 0, 4, buf_2_phys)) {
		// Null terminate the string to safely print it
		((char*)buf_2_vir)[strlen(test_string)] = '\0';
        printf(" [AHCI] Data read from disk: %s.\n", (char*)buf_2_vir);
    } else {
        printf(" [AHCI] Read failed from disk!\n");
    }

    kheap_free((void *)phys_to_vir((uint64_t) buf_1_vir), 0x8000); 	// Free memory
	kheap_free((void *)phys_to_vir((uint64_t) buf_2_vir), 0x8000);	// Free memory

	printf("[AHCI] test completed successfully.\n");

	return;
}











