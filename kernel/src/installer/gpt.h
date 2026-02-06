
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>



// GPT Constants
#define ESP_START_LBA 2048
#define SECTOR_SIZE 512
#define GPT_SIGNATURE "EFI PART"
#define GPT_REVISION 0x00010000
#define GPT_HEADER_LBA 1
#define GPT_ENTRIES_LBA 2
#define GPT_ENTRIES_COUNT 128
#define GPT_ENTRY_SIZE 128


typedef uint8_t guid_t[16];

static const guid_t DISK_GUID_EXAMPLE = {
    0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,
    0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF
};

static const guid_t ESP_TYPE_GUID = {
    0x28,0x73,0x2a,0xc1,0x1f,0xf8,0xd2,0x11,
    0xba,0x4b,0x00,0xa0,0xc9,0x3e,0xc9,0x3b
};

static const guid_t LINUX_FS_GUID = {
    0xaf,0x3d,0xc6,0x0f,0x83,0x84,0x72,0x47,
    0x8e,0x79,0x3d,0x69,0xd8,0x47,0x7d,0xe4
};

// Protective MBR Structure
typedef struct __attribute__((packed)) {
    uint8_t boot_code[440];
    uint32_t disk_signature;
    uint16_t reserved;
    struct __attribute__((packed)) {
        uint8_t status;
        uint8_t chs_start[3];
        uint8_t type;
        uint8_t chs_end[3];
        uint32_t lba_start;
        uint32_t sector_count;
    } partition[4];
    uint16_t signature;
} ProtectiveMBR;


// GPT Partition Entry Structure
typedef struct __attribute__((packed)) {
    uint8_t type_guid[16];
    uint8_t unique_guid[16];
    uint64_t start_lba;
    uint64_t end_lba;
    uint64_t attributes;
    char name[72];  // UTF-16LE encoded, but we'll use ASCII for simplicity
} GPTPartitionEntry;


// GPT Header Structure
typedef struct __attribute__((packed)) {
    char signature[8];        // "EFI PART"
    uint32_t revision;
    uint32_t header_size;
    uint32_t header_crc32;
    uint32_t reserved;
    uint64_t current_lba;
    uint64_t backup_lba;
    uint64_t first_usable_lba;
    uint64_t last_usable_lba;
    uint8_t disk_guid[16];
    uint64_t entries_lba;
    uint32_t entries_count;
    uint32_t entry_size;
    uint32_t entries_crc32;
} GPTHeader;



bool create_esp_and_data_partitions(int disk_no, uint64_t esp_partition_sectors, uint64_t data_partition_sectors, uint64_t total_sectors);










