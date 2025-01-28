
#include "../driver/vga.h"
#include "../limine/limine.h"
#include "../lib/string.h"

#include "boot.h"

#include "disk.h"


extern char *FIRMWARE_TYPE;

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

static volatile struct limine_smbios_request smbios_request = {
    .id = LIMINE_SMBIOS_REQUEST,
    .revision = 3
};


// BIOS interrupt to get disk info
void get_bios_disk_info() {
    if(smbios_request.response != NULL){
        uint64_t revision = smbios_request.response->revision;
        uint64_t entry_32 = (uint64_t)smbios_request.response->entry_32;
        uint64_t entry_64 = (uint64_t)smbios_request.response->entry_64;
        print("SMBIOS info found!\n");
    }else{
        print("No SMBIOS info found!\n");
    }
}

// Limine requests
static volatile struct limine_efi_system_table_request efi_system_table_request = {
    .id = LIMINE_EFI_SYSTEM_TABLE_REQUEST,
    .revision = 2
};

// UEFI disk info (simplified example)
void get_uefi_disk_info() {
    // Limine provides access to the UEFI system table
    if (efi_system_table_request.response != NULL) {
        uint64_t efi_system_table_address = (uint64_t)efi_system_table_request.response->address;

        if (efi_system_table_address != NULL) {
            // Use the UEFI system table to query disk information
            // This is a simplified example; actual implementation requires using UEFI protocols
            print("UEFI System Table found. Disk info retrieval not implemented in this example.\n");
        } else {
            print("UEFI System Table not found!\n");
        }
    } else {
        print("UEFI firmware type not detected!\n");
    }
}

// Get disk info based on firmware type
void get_disk_info() {
    if (strcmp(FIRMWARE_TYPE, "X86BIOS") == 0) {
        get_bios_disk_info();
    } else if (strcmp(FIRMWARE_TYPE, "UEFI32") == 0 || strcmp(FIRMWARE_TYPE, "UEFI64") == 0) {
        get_uefi_disk_info();
    } else {
        print("Unsupported firmware type for disk info retrieval!\n");
    }
}

