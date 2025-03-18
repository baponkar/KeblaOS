#pragma once

#include <stdint.h>


// Define a dummy disk structure.
typedef struct {
    uint32_t controller; // In a real driver, this might be a pointer to an AHCI controller structure.
    uint32_t port;       // Port number on the AHCI controller.
} disk_t;

// Stub function declarations (these must be implemented elsewhere or as stubs for testing).
disk_t* get_disk(int disk_index);
void ahci_read_sector(uint32_t controller, uint32_t port, uint64_t lba, void* buffer);

// Define an AHCI command table structure.
typedef struct {
    struct {
        uint64_t dba;  // Data Base Address.
        uint32_t dbc;  // Data Byte Count (0-indexed).
        // Other fields might follow...
    } prdt_entry[1];   // For simplicity, we assume one PRDT entry.
} ahci_cmd_table_t;

ahci_cmd_table_t* prepare_ahci_command(uint32_t controller, uint32_t port);
void setup_write_command(uint32_t controller, uint32_t port, uint64_t lba, int sector_count, ahci_cmd_table_t* cmd_table);
int ahci_issue_command(uint32_t controller, uint32_t port);

void test_disk_write_read();
void write_sector_to_disk(int disk_index, uint64_t lba, const void* buffer);
void read_sector_from_disk(int disk_index, uint64_t lba, void* buffer);
void get_disk_info();
void get_uefi_disk_info();






