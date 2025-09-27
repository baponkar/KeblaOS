/*
SATAPI CD/DVD Drive Support for AHCI (SATA) Controllers

Reference:
https://wiki.osdev.org/El-Torito
https://wiki.osdev.org/ISO_9660

*/

#include "../../../lib/stdio.h"
#include "../../../lib/string.h"

#include "../../../memory/vmm.h"
#include "../../../memory/kheap.h"

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


#define MAX_SATAPI_DISKS 10

extern pci_device_t *mass_storage_controllers;  // Array to store detected mass storage devices
extern size_t mass_storage_count;               // Counter for mass storage devices

HBA_PORT_T **satapi_disks = NULL;               // Store Different AHCI Disks contexts
int satapi_disks_count = 0;                     // Total disks count


void AtpiPortRebase(HBA_MEM_T *abar, int port_no)
{
    HBA_PORT_T *port = &abar->ports[port_no];
    stopCMD(port);

    // Allocate a physically contiguous region for this port:
    // Make it large enough for CLB (1K), FB (1K), CTBA area (8K), plus margin.
    const size_t ALLOC_SIZE = 64 * 1024; // 64 KiB to be safe
    void *base_virt = (void *) kheap_alloc(ALLOC_SIZE, ALLOCATE_DATA);
    if (!base_virt) {
        printf(" portRebase: kmalloc failed\n");
        return;
    }

    // Convert to physical base (what HBA will use)
    uintptr_t base_phys = vir_to_phys((uintptr_t)base_virt);
    if (base_phys == 0) {
        printf(" portRebase: vir_to_phys returned 0\n");
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

    port->cmd |= HBA_PxCMD_ATAPI;  // Set bit 24 for ATAPI devices

    // Wait a bit for ATAPI mode to take effect
    for (volatile int i = 0; i < 1000; i++);

    // Start command engine after setting CLB/FB/CTBA
    startCMD(port);

    // printf("[AHCI] portRebase: done for port %d (phys base %x)\n", port_no, (unsigned long)base_phys);
}


static bool runAtapiCommand(HBA_PORT_T *port, uint8_t *cdb, size_t cdb_len, uintptr_t buf_phys, uint32_t buf_size, bool write)
{
    port->is = (uint32_t)-1; // clear pending interrupts

    int slot = findCMDSlot(port, 32);
    if (slot == -1) return false;

    // CLB → command header base
    uintptr_t clb_phys = ((uint64_t)port->clb) | ((uint64_t)port->clbu << 32);
    HBA_CMD_HEADER_T *cmd_header_base = (HBA_CMD_HEADER_T*)(uintptr_t)phys_to_vir(clb_phys);
    HBA_CMD_HEADER_T *cmd_header = &cmd_header_base[slot];

    // set up header
    cmd_header->cfl   = sizeof(FIS_REG_H2D_T) / 4;
    cmd_header->w     = write ? 1 : 0;
    cmd_header->a     = 1; // ATAPI
    cmd_header->prdtl = (buf_size > 0) ? 1 : 0; // single PRDT

    // Command table
    uint64_t ctba_phys = ((uint64_t)cmd_header->ctbau << 32) | cmd_header->ctba;
    HBA_CMD_TBL_T *cmd_tbl = (HBA_CMD_TBL_T*)(uintptr_t)phys_to_vir(ctba_phys);
    memset(cmd_tbl, 0, sizeof(HBA_CMD_TBL_T) + (cmd_header->prdtl-1)*sizeof(HBA_PRDT_ENTRY_T));

    // Fill PRDT if data buffer
    if (buf_size > 0) {
        cmd_tbl->prdt_entry[0].dba  = (uint32_t)(buf_phys & 0xFFFFFFFF);
        cmd_tbl->prdt_entry[0].dbau = (uint32_t)(buf_phys >> 32);
        cmd_tbl->prdt_entry[0].dbc  = buf_size - 1;
        cmd_tbl->prdt_entry[0].i    = 1;
    }

    // Fill CFIS
    FIS_REG_H2D_T *cfis = (FIS_REG_H2D_T*)&cmd_tbl->cfis;
    memset(cfis, 0, sizeof(FIS_REG_H2D_T));
    cfis->fis_type = FIS_TYPE_REG_H2D;
    cfis->c = 1;
    cfis->command = ATA_CMD_PACKET;
    cfis->device = 0;

    // Copy CDB into acmd
    memset(cmd_tbl->acmd, 0, 16);
    memcpy(cmd_tbl->acmd, cdb, (cdb_len > 16 ? 16 : cdb_len));

    // Wait until port not busy
    int spin = 0;
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000) spin++;
    if (spin == 1000000) {
        printf(" [SATAPI] Port hung\n");
        return false;
    }

    // Issue command
    port->ci = 1 << slot;

    // Wait for completion
    while (true) {
        if (!(port->ci & (1 << slot))) break;
        if (port->is & HBA_PxIS_TFES) {
            printf(" [SATAPI] Task File Error\n");
            return false;
        }
    }

    if (port->is & HBA_PxIS_TFES) {
        printf(" [SATAPI] TFES after completion\n");
        return false;
    }

    return true;
}



bool init_satapi() {

    printf("[SATAPI] Initializing All SATAPI Disks\n");

    satapi_disks = (HBA_PORT_T **) kheap_alloc(sizeof(HBA_PORT_T *) * MAX_SATAPI_DISKS, ALLOCATE_DATA);
    if(!satapi_disks){
        printf(" satapi_disks allocation failed!\n");
        return false;
    }
    memset(satapi_disks, 0, sizeof(HBA_PORT_T *) * MAX_SATAPI_DISKS);

    if(!mass_storage_controllers){
        printf(" mass_storage_controllers is NULL\n");
        return false;
    }
    if(!mass_storage_controllers_count){
        printf(" mass_storage_controllers_count is 0\n");
        return false;
    }

    for(int controllers_no = 0; controllers_no < mass_storage_controllers_count; controllers_no++){

        pci_device_t device = mass_storage_controllers[controllers_no];
        uint64_t bar5 = (uint64_t)(device.base_address_registers[5] & ~0xF);

        if(bar5 == 0){
            continue;   // skip if BAR5 not set
        } 

        HBA_MEM_T *abar = (HBA_MEM_T *) phys_to_vir(bar5);
        if(!abar){
            continue;   // skip if ABAR is NULL
        }

        // Check each port
        uint32_t pi = abar->pi;
        for (size_t i = 0; i < 32; i++) {
            if (pi & 1) {
                if(checkType((HBA_PORT_T *)&abar->ports[i]) == AHCI_DEV_SATAPI) {
                    printf(" Found SATAPI drive at port %d (controller %d)\n", i, controllers_no);
                    AtpiPortRebase(abar, i);

                    satapi_disks[satapi_disks_count++] = (HBA_PORT_T *)&abar->ports[i]; 
                }
                continue;
            }
            pi >>= 1;
        }
    }

    printf("[SATAPI] Successfully initialized %d SATAPI disk(s)\n", satapi_disks_count);
    return true;
}



bool satapi_inquiry(HBA_PORT_T *port) {
    if(!port) return false;
    uint8_t cdb[12] = {0};
    cdb[0] = ATAPI_CMD_INQUIRY;
    cdb[4] = 36; // allocation length

    void *buf = kheap_alloc(36, ALLOCATE_DATA);
    if (!buf) return false;

    uintptr_t buf_phys = vir_to_phys((uintptr_t)buf);
    if (!runAtapiCommand(port, cdb, 12, buf_phys, 36, false)) {
        printf(" Inquiry failed\n");
        return false;
    }

    uint8_t *resp = (uint8_t*)buf;
    printf(" Vendor: %.8s, Product: %.16s, Revision: %.4s\n",
           &resp[8], &resp[16], &resp[32]);
    return true;
}


















