#include "../lib/stdio.h"
#include "../limine/limine.h"
#include "../lib/string.h"
#include "../bootloader/boot.h"
#include "../driver/io/ports.h"
#include "../ahci/ahci.h"


#include "disk.h"

#define SECTOR_SIZE 512
#define AHCI_IS  0x08


extern char *FIRMWARE_TYPE;

disk_t disk;

disk_t disks[100];

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(0);

static volatile struct limine_smbios_request smbios_request = {
    .id = LIMINE_SMBIOS_REQUEST,
    .revision = 0
};

static volatile struct limine_efi_system_table_request efi_system_table_request = {
    .id = LIMINE_EFI_SYSTEM_TABLE_REQUEST,
    .revision = 0
};


void get_bios_disk_info(void) {
    if (smbios_request.response == NULL) {
        printf("No SMBIOS info found!\n");
        return;
    }
    printf("SMBIOS info found!\n");
}

void get_uefi_disk_info(void) {
    if (efi_system_table_request.response == NULL) {
        printf("UEFI firmware type not detected!\n");
        return;
    }
    uint64_t efi_system_table_address = (uint64_t)efi_system_table_request.response->address;
    if (efi_system_table_address != 0)
        printf("UEFI System Table found. Disk info retrieval not implemented.\n");
    else
        printf("UEFI System Table not found!\n");
}


// Get disk info based on the system firmware type (a system variable)
void get_disk_info(void) {
    if (FIRMWARE_TYPE == NULL) {
        printf("FIRMWARE_TYPE is NULL! Cannot determine disk info.\n");
        return;
    }
    if (strcmp(FIRMWARE_TYPE, "X86BIOS") == 0)
        get_bios_disk_info();
    else if (strcmp(FIRMWARE_TYPE, "UEFI32") == 0 || strcmp(FIRMWARE_TYPE, "UEFI64") == 0)
        get_uefi_disk_info();
    else
        printf("Unsupported firmware type for disk info retrieval!\n");
}


disk_t* get_disk(int disk_index) {
    disk_t disk = disks[disk_index];
    return (disk_t*) &disk;
}

