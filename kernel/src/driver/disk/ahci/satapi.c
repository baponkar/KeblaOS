/*
SATAPI CD/DVD Drive Support for AHCI (SATA) Controllers

Reference:
https://wiki.osdev.org/El-Torito
https://wiki.osdev.org/ISO_9660

*/

#include "../../../lib/stdio.h"
#include "../../../lib/stdlib.h"
#include "../../../lib/string.h"

#include "../../../memory/vmm.h"

#include "satapi.h"



#define ATA_CMD_PACKET          0xA0

// ATAPI Command Opcodes
#define ATAPI_CMD_INQUIRY       0x12
#define ATAPI_CMD_TEST_UNIT     0x00
#define ATAPI_CMD_READ_CAPACITY 0x25    // 
#define ATAPI_CMD_READ10        0x28    // Commonly used for CD/DVD
#define ATAPI_CMD_READ12        0xA8    // Not commonly used for CD/DVD
#define ATAPI_CMD_START_STOP    0x1B

// Each sector on CD-ROM is usually 2048 bytes
#define ATAPI_SECTOR_SIZE       2048

#define HBA_PxCMD_ATAPI (1 << 24)                // Bit 24 in PxCMD register to enable ATAPI mode

void AtpiPortRebase(HBA_PORT_T *port)
{
    stopCMD(port);

    // Set ATAPI mode FIRST, before any command setup
    port->cmd |= HBA_PxCMD_ATAPI;  // Set bit 24 for ATAPI devices
    
    // Wait for ATAPI mode to take effect
    for (volatile int i = 0; i < 1000; i++);

    // Allocate memory (same as your working SATA version)
    const size_t ALLOC_SIZE = 64 * 1024;
    void *base_virt = (void *) malloc(ALLOC_SIZE);
    if (!base_virt) {
        printf("AtpiPortRebase: malloc failed\n");
        return;
    }

    uintptr_t base_phys = vir_to_phys((uintptr_t)base_virt);
    if (base_phys == 0) {
        printf("AtpiPortRebase: vir_to_phys returned 0\n");
        free(base_virt);
        return;
    }

    // Initialize structures (same as your working code)
    uintptr_t clb_phys = base_phys + 0x0;
    void *clb_virt = (void *)((uintptr_t)base_virt + 0x0);
    memset(clb_virt, 0, 0x400);

    uintptr_t fb_phys = base_phys + 0x1000;
    void *fb_virt = (void *)((uintptr_t)base_virt + 0x1000);
    memset(fb_virt, 0, 0x100);

    uintptr_t ctba_base_phys = base_phys + 0x2000;
    void *ctba_base_virt = (void *)((uintptr_t)base_virt + 0x2000);
    memset(ctba_base_virt, 0, 0x2000);

    // Program registers
    port->clb  = (uint32_t)(clb_phys & 0xFFFFFFFF);
    port->clbu = (uint32_t)(clb_phys >> 32);
    port->fb   = (uint32_t)(fb_phys & 0xFFFFFFFF);
    port->fbu  = (uint32_t)(fb_phys >> 32);

    // Initialize command headers - CRITICAL: Set 'a' bit for ATAPI
    HBA_CMD_HEADER_T* cmd_header = (HBA_CMD_HEADER_T*) clb_virt;
    for (int i = 0; i < 32; ++i) {
        uintptr_t phys_ctba = ctba_base_phys + (i * 0x100);
        cmd_header[i].ctba  = (uint32_t)(phys_ctba & 0xFFFFFFFF);
        cmd_header[i].ctbau = (uint32_t)(phys_ctba >> 32);
        cmd_header[i].prdtl = 8;
        cmd_header[i].a     = 1;  // ATAPI command flag
        cmd_header[i].cfl   = 5;  // FIS size in DWORDS
        
        void* virt_ctba = (void*)((uintptr_t)ctba_base_virt + (i * 0x100));
        memset(virt_ctba, 0, 0x100);
    }

    startCMD(port);
}


bool runAtapiCommand(HBA_PORT_T *port, uint8_t *cdb, size_t cdb_len, 
                     uintptr_t buf_phys, uint32_t buf_size, bool write)
{
    port->is = (uint32_t)-1;

    int slot = findCMDSlot(port, 32);
    if (slot == -1) return false;

    // Get command header
    uintptr_t clb_phys = ((uint64_t)port->clb) | ((uint64_t)port->clbu << 32);
    HBA_CMD_HEADER_T *cmd_header_base = (HBA_CMD_HEADER_T*)(uintptr_t)phys_to_vir(clb_phys);
    HBA_CMD_HEADER_T *cmd_header = &cmd_header_base[slot];

    // Setup command header for ATAPI
    cmd_header->cfl   = 5;        // FIS_REG_H2D size in DWORDS
    cmd_header->a     = 1;        // ATAPI command
    cmd_header->w     = write ? 1 : 0;
    cmd_header->prdtl = (buf_size > 0) ? 1 : 0;

    // Command table
    uint64_t ctba_phys = ((uint64_t)cmd_header->ctbau << 32) | cmd_header->ctba;
    HBA_CMD_TBL_T *cmd_tbl = (HBA_CMD_TBL_T*)(uintptr_t)phys_to_vir(ctba_phys);
    
    size_t cmdtbl_size = sizeof(HBA_CMD_TBL_T) + (cmd_header->prdtl - 1) * sizeof(HBA_PRDT_ENTRY_T);
    memset(cmd_tbl, 0, cmdtbl_size);

    // Setup PRDT if we have data transfer
    if (buf_size > 0) {
        cmd_tbl->prdt_entry[0].dba  = (uint32_t)(buf_phys & 0xFFFFFFFF);
        cmd_tbl->prdt_entry[0].dbau = (uint32_t)(buf_phys >> 32);
        cmd_tbl->prdt_entry[0].dbc  = buf_size - 1;
        cmd_tbl->prdt_entry[0].i    = 1;
    }

    // Setup FIS for ATAPI PACKET command
    FIS_REG_H2D_T *cfis = (FIS_REG_H2D_T*)&cmd_tbl->cfis;
    memset(cfis, 0, sizeof(FIS_REG_H2D_T));
    cfis->fis_type = FIS_TYPE_REG_H2D;
    cfis->c        = 1;           // Command
    cfis->command  = ATA_CMD_PACKET;
    
    // ATAPI requires feature register to be set
    cfis->featurel = write ? 0x00 : 0x01;  // 0x01 for DMA read, 0x00 for DMA write
    
    // For ATAPI, count register contains the transfer length in sectors (2048-byte blocks)
    uint32_t sector_count = (buf_size + 2047) / 2048;
    cfis->countl = sector_count & 0xFF;
    cfis->counth = (sector_count >> 8) & 0xFF;

    // Copy CDB
    memset(cmd_tbl->acmd, 0, 16);
    memcpy(cmd_tbl->acmd, cdb, (cdb_len > 16 ? 16 : cdb_len));

    // Wait for port ready
    int spin = 0;
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000) 
        spin++;
    
    if (spin >= 1000000) {
        printf(" [SATAPI] Port not ready\n");
        return false;
    }

    // Issue command
    port->ci = 1 << slot;

    // Wait for completion
    while (port->ci & (1 << slot)) {
        if (port->is & HBA_PxIS_TFES) {
            printf(" [SATAPI] Task File Error: TFD=%x\n", port->tfd);
            return false;
        }
    }

    // Check for errors after completion
    if (port->is & HBA_PxIS_TFES) {
        printf(" [SATAPI] Task File Error after completion: TFD=0x%x\n", port->tfd);
        return false;
    }

    return true;
}

bool satapi_inquiry(HBA_PORT_T *port) {
    if(!port) return false;
    uint8_t cdb[12] = {0};
    cdb[0] = ATAPI_CMD_INQUIRY;
    cdb[4] = 36; // allocation length

    void *buf = malloc(36);
    if (!buf) return false;
    memset(buf, 0, 36);

    uintptr_t buf_phys = vir_to_phys((uintptr_t)buf);
    if (!runAtapiCommand(port, cdb, 12, buf_phys, 36, false)) {
        printf("Inquiry failed\n");
        free(buf);
        return false;
    }

    uint8_t *resp = (uint8_t*)buf;
    
    // Extract strings properly (they're not null-terminated in the response)
    char vendor[9] = {0};
    char product[17] = {0};
    char revision[5] = {0};
    
    memcpy(vendor, &resp[8], 8);
    memcpy(product, &resp[16], 16);
    memcpy(revision, &resp[32], 4);
    
    // Trim trailing spaces
    for(int i = 7; i >= 0 && vendor[i] == ' '; i--) vendor[i] = 0;
    for(int i = 15; i >= 0 && product[i] == ' '; i--) product[i] = 0;
    for(int i = 3; i >= 0 && revision[i] == ' '; i--) revision[i] = 0;
    
    printf("SATAPI Device Found:\n");
    printf("  Vendor: '%s'\n", vendor);
    printf("  Product: '%s'\n", product);
    printf("  Revision: '%s'\n", revision);
    
    // Also print device type
    uint8_t peripheral_type = resp[0] & 0x1F;
    printf("  Device Type: %x", peripheral_type);
    switch(peripheral_type) {
        case 0x05: printf(" (CD/DVD drive)\n"); break;
        case 0x00: printf(" (Direct access device)\n"); break;
        case 0x07: printf(" (Optical memory device)\n"); break;
        default: printf(" (Unknown)\n"); break;
    }
    
    free(buf);
    return true;
}




bool satapi_read(HBA_PORT_T *port, uint32_t lba, uint32_t sector_count, void *buffer) {
    if (!port || !buffer || sector_count == 0) return false;
    
    // ATAPI uses 2048-byte sectors for CD/DVD
    const uint32_t SECTOR_SIZE = 2048;
    uint32_t transfer_size = sector_count * SECTOR_SIZE;
    
    // Use READ(12) command for CD/DVD
    uint8_t cdb[12] = {0};
    cdb[0] = 0xA8; // READ(12) command
    cdb[2] = (lba >> 24) & 0xFF; // LBA MSB
    cdb[3] = (lba >> 16) & 0xFF;
    cdb[4] = (lba >> 8) & 0xFF;
    cdb[5] = lba & 0xFF;         // LBA LSB
    cdb[6] = (sector_count >> 24) & 0xFF; // Transfer length MSB
    cdb[7] = (sector_count >> 16) & 0xFF;
    cdb[8] = (sector_count >> 8) & 0xFF;
    cdb[9] = sector_count & 0xFF; // Transfer length LSB
    
    uintptr_t buf_phys = vir_to_phys((uintptr_t)buffer);
    if (buf_phys == 0) {
        printf("SATAPI Read: Failed to get physical address\n");
        return false;
    }
    
    // printf("SATAPI Read: LBA=%u, sectors=%u, size=%u bytes\n", 
    //        lba, sector_count, transfer_size);
    
    return runAtapiCommand(port, cdb, 12, buf_phys, transfer_size, false);
}


bool satapi_write(HBA_PORT_T *port, uint32_t lba, uint32_t sector_count, void *buffer) {
    if (!port || !buffer || sector_count == 0) return false;
    
    // ATAPI uses 2048-byte sectors for CD/DVD
    const uint32_t SECTOR_SIZE = 2048;
    uint32_t transfer_size = sector_count * SECTOR_SIZE;
    
    // Use WRITE(12) command
    uint8_t cdb[12] = {0};
    cdb[0] = 0xAA; // WRITE(12) command
    cdb[2] = (lba >> 24) & 0xFF; // LBA MSB
    cdb[3] = (lba >> 16) & 0xFF;
    cdb[4] = (lba >> 8) & 0xFF;
    cdb[5] = lba & 0xFF;         // LBA LSB
    cdb[6] = (sector_count >> 24) & 0xFF; // Transfer length MSB
    cdb[7] = (sector_count >> 16) & 0xFF;
    cdb[8] = (sector_count >> 8) & 0xFF;
    cdb[9] = sector_count & 0xFF; // Transfer length LSB
    
    uintptr_t buf_phys = vir_to_phys((uintptr_t)buffer);
    if (buf_phys == 0) {
        printf("SATAPI Write: Failed to get physical address\n");
        return false;
    }
    
    // printf("SATAPI Write: LBA=%u, sectors=%u, size=%u bytes\n", 
    //        lba, sector_count, transfer_size);
    
    return runAtapiCommand(port, cdb, 12, buf_phys, transfer_size, true);
}



// Check if media is present
bool satapi_check_media(HBA_PORT_T *port) {
    uint8_t cdb[12] = {0};
    cdb[0] = 0x46; // GET CONFIGURATION command
    cdb[7] = 0x00; // Starting feature number
    cdb[8] = 0x02; // Allocation length MSB
    cdb[9] = 0x00; // Allocation length LSB (512 bytes)
    
    void *buf = malloc(512);
    if (!buf) return false;
    memset(buf, 0, 512);
    
    uintptr_t buf_phys = vir_to_phys((uintptr_t)buf);
    bool success = runAtapiCommand(port, cdb, 12, buf_phys, 512, false);
    
    if (success) {
        uint8_t *resp = (uint8_t*)buf;
        printf("Media present: %s\n", (resp[2] & 0x02) ? "Yes" : "No");
    }
    
    free(buf);
    return success;
}

uint16_t satapi_get_bytes_per_sector(HBA_PORT_T *port){
    uint8_t cdb[10] = {0};
    cdb[0] = 0x25; // Read Capacity Command

    void *buf = malloc(8);
    if(!buf) return 0;
    memset(buf, 0, 8);

    uintptr_t buf_phys = (uintptr_t)vir_to_phys((uint64_t)buf);
    bool success = runAtapiCommand(port, cdb, 10, buf_phys, 8, false);

    uint32_t bytes_per_sector = 0;

    if(success){
        uint32_t *data = (uint32_t*)buf;
        bytes_per_sector = __builtin_bswap32(data[1]); // Convert BE → LE
        printf("SATAPI: Bytes per sector raw: %x -> %u\n", data[1], bytes_per_sector);
    } else {
        printf("SATAPI: READ CAPACITY failed\n");
    }

    free(buf);  // FIXED: Use actual allocated size
    return bytes_per_sector;
}

uint64_t satapi_get_total_sectors(HBA_PORT_T *port){
    uint8_t cdb[10] = {0};
    cdb[0] = 0x25; // Read Capacity Command

    void *buf = malloc(8);
    if(!buf) return 0;
    memset(buf, 0, 8);

    uintptr_t buf_phys = (uintptr_t)vir_to_phys((uint64_t)buf);
    bool success = runAtapiCommand(port, cdb, 10, buf_phys, 8, false);

    uint64_t total_sectors = 0;

    if(success){
        uint32_t *data = (uint32_t*)buf;
        // FIXED: Use data[0] for last LBA, then +1 for total sectors
        total_sectors = __builtin_bswap32(data[0]) + 1;
        printf("SATAPI: Total sectors raw: %x -> %u\n", data[0], total_sectors);
    } else {
        printf("SATAPI: READ CAPACITY failed\n");
    }

    free(buf);  // FIXED: Use actual allocated size
    return total_sectors;
}

// Read capacity (get total sectors and sector size)
bool satapi_read_capacity(HBA_PORT_T *port, uint32_t *last_lba, uint32_t *sector_size) {
    uint8_t cdb[10] = {0};
    cdb[0] = 0x25; // READ CAPACITY command
    
    void *buf = malloc(8);
    if (!buf) return false;
    memset(buf, 0, 8);
    
    uintptr_t buf_phys = vir_to_phys((uintptr_t)buf);
    bool success = runAtapiCommand(port, cdb, 10, buf_phys, 8, false);
    
    if (success) {
        uint32_t *data = (uint32_t*)buf;
        *last_lba = __builtin_bswap32(data[0]); // Big-endian to little-endian
        *sector_size = __builtin_bswap32(data[1]);
        
        printf("SATAPI Capacity: Last LBA=%u, Sector Size=%u bytes\n", 
               *last_lba, *sector_size);
    }
    
    free(buf);
    return success;
}


bool satapi_eject(HBA_PORT_T *port) {
    uint8_t cdb[12];
    memset(cdb, 0, sizeof(cdb));

    cdb[0] = 0x1B;   // START STOP UNIT
    cdb[4] = 0x02;   // LOEJ=1, START=0 (eject tray)

    printf("[SATAPI] Sending eject command...\n");

    bool success = runAtapiCommand(port, cdb, sizeof(cdb), 0, 0, false);
    if (success)
        printf("[SATAPI] Tray ejected successfully.\n");
    else
        printf("[SATAPI] Eject command failed.\n");

    return success;
}

bool satapi_load(HBA_PORT_T *port) {
    uint8_t cdb[12];
    memset(cdb, 0, sizeof(cdb));

    cdb[0] = 0x1B;   // START STOP UNIT
    cdb[4] = 0x03;   // LOEJ=1, START=1 (load tray)

    printf("[SATAPI] Sending load (close tray) command...\n");

    bool success = runAtapiCommand(port, cdb, sizeof(cdb), 0, 0, false);
    if (success)
        printf("[SATAPI] Tray loaded successfully.\n");
    else
        printf("[SATAPI] Load command failed.\n");

    return success;
}



void test_satapi(HBA_PORT_T *port) {
    // First check what device we have
    if (!satapi_inquiry(port)) {
        printf("SATAPI Inquiry failed\n");
        return;
    }
    
    // Check media
    satapi_check_media(port);
    
    // Read capacity
    uint32_t last_lba, sector_size;
    if (satapi_read_capacity(port, &last_lba, &sector_size)) {
        // Read first sector
        void *buffer = malloc(sector_size);
        if (buffer && satapi_read(port, 0, 1, buffer)) {
            printf("Successfully read first sector\n");
            // Process CD/DVD sector data here...
        }
        free(buffer);
    }
}




