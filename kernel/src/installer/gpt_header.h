#pragma once

#include <stdint.h>
#include <string.h>
#include <stdbool.h>



#pragma pack(push, 1)
typedef struct {
    uint64_t signature;              // "EFI PART" (45h 46h 49h 20h 50h 41h 52h 54h)
    uint32_t revision;               // For GPT version 1.0, this is 0x00010000
    uint32_t header_size;            // Size of the header in bytes (usually 92)
    uint32_t header_crc32;           // CRC32 checksum of the header
    uint32_t reserved;               // Must be 0
    uint64_t current_lba;            // LBA of this header copy
    uint64_t backup_lba;             // LBA of the alternate header
    uint64_t first_usable_lba;       // First usable LBA for partitions (after header + entries)
    uint64_t last_usable_lba;        // Last usable LBA for partitions (before backup header)
    uint8_t  disk_guid[16];          // Unique disk GUID
    uint64_t partition_entries_lba;  // Starting LBA of array of partition entries
    uint32_t num_partition_entries;  // Number of partition entries (usually 128)
    uint32_t partition_entry_size;   // Size of each partition entry (usually 128)
    uint32_t partition_entries_crc32;// CRC32 of partition entries array
    uint8_t  reserved2[420];         // Padding to 512 bytes (sector size)
} gpt_header_t;
#pragma pack(pop)


#pragma pack(push, 1)
typedef struct {
    uint8_t  partition_type_guid[16]; // Partition type GUID
    uint8_t  unique_partition_guid[16];// Unique partition GUID
    uint64_t starting_lba;            // Starting LBA of the partition
    uint64_t ending_lba;              // Ending LBA of the partition
    uint64_t attributes;              // Attribute flags
    uint16_t partition_name[36];      // Partition name (UTF-16)
} gpt_partition_entry_t;
#pragma pack(pop)


#pragma pack(push, 1)
typedef struct {
    uint8_t  boot_indicator;
    uint8_t  starting_chs[3];
    uint8_t  partition_type;
    uint8_t  ending_chs[3];
    uint32_t starting_lba;
    uint32_t size_in_lba;
} mbr_partition_entry_t;
#pragma pack(pop)


#pragma pack(push, 1)
typedef struct {
    uint8_t  bootstrap[446];
    mbr_partition_entry_t partitions[4];
    uint16_t signature;
} protective_mbr_t;
#pragma pack(pop)


bool create_complete_gpt(int disk_no);


















