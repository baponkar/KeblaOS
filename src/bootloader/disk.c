
#include "../lib/stdio.h"
#include "../limine/limine.h"
#include "../lib/string.h"

#include "boot.h"

#include "disk.h"


extern char *FIRMWARE_TYPE;

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(0);

static volatile struct limine_smbios_request smbios_request = {
    .id = LIMINE_SMBIOS_REQUEST,
    .revision = 0
};


// BIOS interrupt to get disk info
void get_bios_disk_info() {
    if(smbios_request.response == NULL){
        printf("No SMBIOS info found!\n");
        return;
    }
    uint64_t revision = smbios_request.response->revision;
    uint64_t entry_32 = (uint64_t)smbios_request.response->entry_32;
    uint64_t entry_64 = (uint64_t)smbios_request.response->entry_64;
    printf("SMBIOS info found!\n");
}

// Limine requests
static volatile struct limine_efi_system_table_request efi_system_table_request = {
    .id = LIMINE_EFI_SYSTEM_TABLE_REQUEST,
    .revision = 0
};

// UEFI disk info (simplified example)
void get_uefi_disk_info() {
    // Limine provides access to the UEFI system table
    if (efi_system_table_request.response == NULL) {
        printf("UEFI firmware type not detected!\n");
        return;
    }
    uint64_t efi_system_table_address = (uint64_t)efi_system_table_request.response->address;

    if (efi_system_table_address != NULL) {
        // Use the UEFI system table to query disk information
        // This is a simplified example; actual implementation requires using UEFI protocols
        printf("UEFI System Table found. Disk info retrieval not implemented in this example.\n");
    } else {
        printf("UEFI System Table not found!\n");
    }
}

// Get disk info based on firmware type
void get_disk_info() {
    if (FIRMWARE_TYPE == NULL) {
        printf("FIRMWARE_TYPE is NULL! Cannot determine disk info.\n");
        return;
    }

    if (strcmp(FIRMWARE_TYPE, "X86BIOS") == 0) {
        get_bios_disk_info();
    } else if (strcmp(FIRMWARE_TYPE, "UEFI32") == 0 || strcmp(FIRMWARE_TYPE, "UEFI64") == 0) {
        get_uefi_disk_info();
    } else {
        printf("Unsupported firmware type for disk info retrieval!\n");
    }
}

