


/*
GPT (GUID Partition Table) Partitioned Dissk Sector Structure

Sector 1: Primary GPT Header
- 8 bytes: Signature ("EFI PART")
- 4 bytes: Revision
- 4 bytes: Header size
- 4 bytes: Header CRC32
- 4 bytes: Reserved
- 8 bytes: Current LBA (1)
- 8 bytes: Backup LBA (last LBA of the disk)
- 8 bytes: First usable LBA for partitions
- 8 bytes: Last usable LBA for partitions
- 16 bytes: Disk GUID
- 8 bytes: Starting LBA of partition entries array (usually LBA 2)
- 4 bytes: Number of partition entries
- 4 bytes: Size of a single partition entry (usually 128 bytes)
- 4 bytes: CRC32 of partition entries array

Sector 2-33: Partition Entries (128 entries, 128 bytes each)
- Each entry contains:
  - 16 bytes: Partition type GUID
  - 16 bytes: Unique partition GUID
  - 8 bytes: Starting LBA
  - 8 bytes: Ending LBA
  - 8 bytes: Attributes
  - 72 bytes: Partition name (UTF-16LE)
  
Sector 34: Start of usable partitions
...
Sector n-34: End of usable partitions

Sector n-33 to n-2: Backup Partition Entries (same as primary)
Sector n-1: Backup GPT Header (same as primary, but with current and backup LBA swapped)

Here n = total_sectors - 1

============================================================================================

TWO PARTITIONS ARE CREATED IN THIS EXAMPLE:
┌─────────────────────────────────────────────────────────────┐
│ Sector 0: PROTECTIVE MBR                                    │
├─────────────────────────────────────────────────────────────┤
│ Sector 1: PRIMARY GPT HEADER                                │
├─────────────────────────────────────────────────────────────┤
│ Sectors 2-33: PARTITION ENTRIES (entries for ESP + FS)      │
├─────────────────────────────────────────────────────────────┤
│ Sectors 34-2047: UNUSED/GAP (1MB alignment space)           │
├─────────────────────────────────────────────────────────────┤
│ Sectors 2048-206847: PARTITION 1 - ESP (100MB, FAT32)       │
├─────────────────────────────────────────────────────────────┤
│ Sectors 206848-(n-34): PARTITION 2 - YOUR FILESYSTEM        │
│                           (All remaining space)             │
├─────────────────────────────────────────────────────────────┤
│ Sectors (n-33)-(n-2): BACKUP PARTITION ENTRIES              │
├─────────────────────────────────────────────────────────────┤
│ Sector n-1: BACKUP GPT HEADER                               │
└─────────────────────────────────────────────────────────────┘
*/

#include "../../../lib/string.h"
#include "../../../lib/stdlib.h"
#include "../../../lib/stdio.h"
#include "../../../lib/ctype.h"

#include "../include/diskio.h"

#include "../include/mbr.h"


#include "../include/gpt.h"



// Cyclic Redundancy Check
// CRC32 Calculation for GPT
uint32_t crc32(const void *data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    const uint8_t *bytes = (const uint8_t *)data;
    
    for (size_t i = 0; i < length; i++) {
        crc ^= bytes[i];    // bitwise XOR with crc and bytes[i]
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        }
    }

    return ~crc;
}



GPTPartitionEntry *create_gpt_partition_entry(
    uint8_t type_guid[16], 
    uint8_t unique_guid[16], 
    uint64_t start_lba, 
    uint64_t end_lba, 
    uint64_t attributes, 
    const char* name) {

    GPTPartitionEntry *entry = malloc(sizeof(GPTPartitionEntry));
    if(entry == NULL){
        printf("[PARTITION] Failed to allocate memory for GPT Partition Entry!\n");
        return NULL;
    }
    memset(entry, 0, sizeof(GPTPartitionEntry));
    
    memcpy(entry->type_guid, type_guid, 16);
    memcpy(entry->unique_guid, unique_guid, 16);

    entry->start_lba = start_lba;
    entry->end_lba = end_lba;

    entry->attributes = attributes;
    
    for (size_t i = 0; i < 36 && name[i] != '\0'; i++) {
        entry->name[i * 2] = name[i];     // UTF-16LE: ASCII char in low byte
        // High byte remains 0 from memset
    }
    
    return entry;
}



/*
This function will create both primary and backup GPT headers, write them to disk. 
It will not write partition entries to disk, it will only create headers and write them to disk.
It will also calculate CRC32 for both headers and entries and update the headers accordingly.
 It returns true on success and false on failure.
*/
bool create_gpt_header(
    uint64_t tot_sectors, 
    GPTHeader *primary_header, 
    GPTHeader *backup_header,
    const guid_t disk_guid, 
    GPTPartitionEntry *partitions) {
    
    if(partitions == NULL){
        printf("[PARTITION] GPT Partition entries pointer is NULL for disk!\n");
        return false;
    }

    if(tot_sectors < 128){
        printf("[PARTITION] Disk has less than 128 sectors!\n");

        return false;
    }
    
    if(!primary_header || !backup_header){
        printf("[PARTITION] Primary or Backup GPT header pointer is NULL for disk!\n");
        return false;
    }

    // Creating Primary Header
    memset(primary_header, 0, sizeof(GPTHeader));
    memcpy(primary_header->signature, "EFI PART", 8);
    primary_header->revision = 0x00010000;  // Version 1.0
    primary_header->header_size = sizeof(GPTHeader);
    primary_header->header_crc32 = 0;       // Temporarily 0 for CRC calculation it will be calculated after partition entries are created and CRC32 is calculated over entries
    primary_header->reserved = 0;
    primary_header->current_lba = 1;        // Primary GPT header is at LBA 1
    primary_header->backup_lba = tot_sectors - 1;  // Backup GPT header at last LBA
    uint64_t entry_sectors = (GPT_ENTRIES_COUNT * sizeof(GPTPartitionEntry) + SECTOR_SIZE - 1) / SECTOR_SIZE; // Should be 32 sectors for 128 entries of 128 bytes each
    primary_header->first_usable_lba = entry_sectors + 2;                   // mbr(0) + primary gpt(1) + partition entries(2-33)
    primary_header->last_usable_lba = tot_sectors - entry_sectors - 2;      // before backup entries and backup gpt
    memcpy(primary_header->disk_guid, disk_guid, 16);
    primary_header->entries_lba = GPT_ENTRIES_START_LBA;        // Partition entries start at LBA 2
    primary_header->entries_count = GPT_ENTRIES_COUNT;
    primary_header->entry_size = GPT_ENTRY_SIZE;

    // Creating Backup Header
    memset(backup_header, 0, sizeof(GPTHeader));
    memcpy(backup_header->signature, "EFI PART", 8);
    backup_header->revision = 0x00010000;  // Version 1.0
    backup_header->header_size = sizeof(GPTHeader);
    backup_header->header_crc32 = 0;        // Temporarily 0 for CRC calculation
    backup_header->reserved = 0;
    backup_header->current_lba = tot_sectors - 1;  // Backup GPT header at last LBA
    backup_header->backup_lba = 1;          // Primary GPT header is at LBA 1
    backup_header->first_usable_lba = entry_sectors + 2;                    // mbr(0) + primary gpt(1) + partition entries(2-33)
    backup_header->last_usable_lba = tot_sectors - entry_sectors - 2;       // before backup entries and backup gpt
    memcpy(backup_header->disk_guid, disk_guid, 16);
    backup_header->entries_lba = tot_sectors - entry_sectors - 1;
    backup_header->entries_count = GPT_ENTRIES_COUNT;
    backup_header->entry_size = GPT_ENTRY_SIZE;

    // Calculate CRC32 of the headers (excluding CRC32 field itself)
    primary_header->header_crc32 = 0;
    backup_header->header_crc32 = 0;

    // Calculate CRC32 of partition entries
    uint32_t entries_crc = crc32(partitions, GPT_ENTRIES_COUNT * GPT_ENTRY_SIZE);
    primary_header->entries_crc32 = entries_crc;
    backup_header->entries_crc32 = entries_crc;

    
    
    primary_header->header_crc32 = crc32(primary_header, primary_header->header_size);
    backup_header->header_crc32 = crc32(backup_header, backup_header->header_size);

    uint8_t temp_buffer[512];
    memset(temp_buffer, 0, sizeof(temp_buffer));

    memcpy(temp_buffer, primary_header, sizeof(GPTHeader));
    // Writing primary GPT header to disk
    if(!fat32_disk_write( 1, 1, temp_buffer)) {
        printf("[PARTITION] Failed to write primary GPT header for disk!\n");
        return false;
    }

    // Write backup GPT header to disk
    if (!fat32_disk_write( tot_sectors - 1, 1, backup_header)) {
        printf("[PARTITION] Failed to write backup GPT header for disk!\n");
        return false;
    }

    return true;
}


