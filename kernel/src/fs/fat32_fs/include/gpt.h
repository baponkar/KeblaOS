
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "guid.h"


// GPT Constants
#define ESP_START_LBA 2048          
#define SECTOR_SIZE 512

#define GPT_SIGNATURE "EFI PART"
#define GPT_REVISION 0x00010000     // GPT version is 1.0
#define GPT_HEADER_LBA 1            // GPT Header position at LBA 1
#define GPT_ENTRIES_START_LBA 2     // GPT Entry starts from LBA 2 to 32
#define GPT_ENTRIES_COUNT 128       // Total GPT Entries available in GPT Disk
#define GPT_ENTRY_SIZE 128          // Every GPT Entry size is 128 bytes


// GPT Partition Entry Structure
typedef struct __attribute__((packed)) {
    uint8_t type_guid[16];
    uint8_t unique_guid[16];
    uint64_t start_lba;
    uint64_t end_lba;
    uint64_t attributes;
    char name[72];      // UTF-16LE encoded, but we'll use ASCII for simplicity
} GPTPartitionEntry;    // 128 bytes


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
} GPTHeader;    // 92 bytes


uint32_t crc32(const void *data, size_t length);

GPTPartitionEntry *create_gpt_partition_entry(
    uint8_t type_guid[16], 
    uint8_t unique_guid[16], 
    uint64_t start_lba, 
    uint64_t end_lba, 
    uint64_t attributes, 
    const char* name
);

bool create_gpt_header(
    uint64_t tot_sectors, 
    GPTHeader *primary_header, 
    GPTHeader *backup_header,
    const guid_t disk_guid, 
    GPTPartitionEntry *partitions
);











