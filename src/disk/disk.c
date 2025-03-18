#include "../lib/stdio.h"
#include "../limine/limine.h"
#include "../lib/string.h"
#include "../bootloader/boot.h"

#include "disk.h"

extern char *FIRMWARE_TYPE;

#define SECTOR_SIZE 512
static char simulated_disk_sector[SECTOR_SIZE];

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

//
// BIOS disk info via SMBIOS
//
void get_bios_disk_info(void) {
    if (smbios_request.response == NULL) {
        printf("No SMBIOS info found!\n");
        return;
    }
    printf("SMBIOS info found!\n");
}

//
// UEFI disk info (simplified)
//
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

//
// Get disk info based on the system firmware type (a system variable)
//
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

//
// Read a sector from disk via AHCI driver routines
//
void read_sector_from_disk(int disk_index, uint64_t lba, void* buffer) {
    disk_t* disk = get_disk(disk_index);
    if (!disk) {
        printf("No disk found at index %d\n", disk_index);
        return;
    }
    ahci_read_sector(disk->controller, disk->port, lba, buffer);
}

//
// Test function: Write a sector then read it back and compare
//
void test_disk_write_read(void) {
    char write_buffer[SECTOR_SIZE];
    char read_buffer[SECTOR_SIZE];

    memset(write_buffer, 0, SECTOR_SIZE);
    memset(read_buffer, 0, SECTOR_SIZE);

    const char *test_str = "Hello Disk!";
    strncpy(write_buffer, test_str, SECTOR_SIZE);

    write_sector_to_disk(0, 0, write_buffer);
    read_sector_from_disk(0, 0, read_buffer);

    if (memcmp(write_buffer, read_buffer, SECTOR_SIZE) == 0)
        printf("Disk write/read test passed.\n");
    else
        printf("Disk write/read test failed!\n");
}


/*=== Stub Implementations ===*/

disk_t* get_disk(int disk_index) {
    // For simulation, always return a dummy disk.
    static disk_t dummy_disk = { .controller = 1, .port = 1 };
    return &dummy_disk;
}

void ahci_read_sector(uint32_t controller, uint32_t port, uint64_t lba, void* buffer) {
    // For simulation, copy the simulated disk sector.
    memcpy(buffer, simulated_disk_sector, SECTOR_SIZE);
}

ahci_cmd_table_t* prepare_ahci_command(uint32_t controller, uint32_t port) {
    static ahci_cmd_table_t cmd_table;
    return &cmd_table;
}

void setup_write_command(uint32_t controller, uint32_t port, uint64_t lba, int sector_count, ahci_cmd_table_t* cmd_table) {
    // In a real driver, fill in command details for a write command.
}

int ahci_issue_command(uint32_t controller, uint32_t port) {
    // Simulate success.
    return 1;
}

void write_sector_to_disk(int disk_index, uint64_t lba, const void* buffer) {
    disk_t* disk = get_disk(disk_index);
    if (!disk) {
        printf("No disk found at index %d\n", disk_index);
        return;
    }
    
    ahci_cmd_table_t* cmd_table = prepare_ahci_command(disk->controller, disk->port);
    cmd_table->prdt_entry[0].dba = (uint64_t)buffer;
    cmd_table->prdt_entry[0].dbc = SECTOR_SIZE - 1;
    
    setup_write_command(disk->controller, disk->port, lba, 1, cmd_table);
    
    if (!ahci_issue_command(disk->controller, disk->port)) {
        printf("Write command failed on disk %d at LBA %llu\n", disk_index, lba);
        return;
    }
    // Simulate writing by copying the buffer into our simulated disk sector.
    memcpy(simulated_disk_sector, buffer, SECTOR_SIZE);
}
