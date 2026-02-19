/*
GPT (GUID Partition Table) Partitioned Dissk Sector Structure

Sector 0: Protective MBR
- 446 bytes: Bootstrap code
- 64 bytes: 4 Partition entries (16 bytes each)
- 2 bytes: MBR Signature (0x55AA)

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




#include "../driver/disk/disk.h"

#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../lib/stdlib.h"

#include "gpt.h"


// CRC32 Calculation for GPT
static uint32_t crc32(const void *data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    const uint8_t *bytes = (const uint8_t *)data;
    
    for (size_t i = 0; i < length; i++) {
        crc ^= bytes[i];
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        }
    }
    
    return ~crc;
}


// Create Protective MBR
static void create_protective_mbr(ProtectiveMBR *mbr, uint64_t total_sectors) {
    memset(mbr, 0, sizeof(ProtectiveMBR));
    
    // Create a protective partition entry
    mbr->partition[0].type = 0xEE;  // Protective MBR type
    mbr->partition[0].lba_start = 1;
    
    // Set partition size (limited to 32-bit in MBR)
    uint32_t mbr_sectors = (total_sectors > 0xFFFFFFFFULL) ? 0xFFFFFFFF : (uint32_t)(total_sectors - 1);
    mbr->partition[0].sector_count = mbr_sectors;   // Fake Partition of whole disk
    
    // MBR signature
    mbr->signature = 0xAA55;
}

static void set_partition_name_utf16(GPTPartitionEntry *entry, const char *name) {
    memset(entry->name, 0, 72);
    
    size_t i;
    for (i = 0; i < 36 && name[i] != '\0'; i++) {
        entry->name[i * 2] = name[i];     // UTF-16LE: ASCII char in low byte
        // High byte remains 0 from memset
    }
}


static GPTPartitionEntry create_partition_entry(uint8_t type_guid[16], uint8_t unique_guid[16], uint64_t start_lba, uint64_t end_lba, uint64_t attributes, const char* name) {
    GPTPartitionEntry entry;
    memset(&entry, 0, sizeof(GPTPartitionEntry));
    
    memcpy(entry.type_guid, type_guid, 16);
    memcpy(entry.unique_guid, unique_guid, 16);
    entry.start_lba = start_lba;
    entry.end_lba = end_lba;
    entry.attributes = attributes;

    set_partition_name_utf16(&entry, name);
    
    return entry;
}

// This function creates both primary and backup GPT headers
static GPTHeader *create_primary_gpt_header(int disk_no, uint64_t total_sectors) {
    GPTHeader *primary_header = malloc(sizeof(GPTHeader));
    memset(primary_header, 0, sizeof(GPTHeader));
    
    // GPT Header fields
    memcpy(primary_header->signature, "EFI PART", 8);
    primary_header->revision = 0x00010000;  // Version 1.0
    primary_header->header_size = sizeof(GPTHeader);
    
    primary_header->header_crc32 = 0; // Temporarily 0 for CRC calculation

    primary_header->reserved = 0;

    primary_header->current_lba = 1;  // Primary GPT header is at LBA 1
    primary_header->backup_lba = total_sectors - 1;  // Backup GPT header at last LBA
    

    uint64_t entry_sectors = (int) (GPT_ENTRIES_COUNT * sizeof(GPTPartitionEntry) + SECTOR_SIZE - 1) / SECTOR_SIZE; // Should be 32 sectors for 128 entries of 128 bytes each
    
    // Set partition entries info
    primary_header->first_usable_lba = entry_sectors + 2;                   // mbr(0) + primary gpt(1) + partition entries(2-33)
    primary_header->last_usable_lba = total_sectors - entry_sectors - 2;    // before backup entries and backup gpt

    memcpy(primary_header->disk_guid, DISK_GUID_EXAMPLE, 16); // Example GUID
    primary_header->entries_lba = 2;  // Partition entries start at LBA 2
    primary_header->entries_count = GPT_ENTRIES_COUNT;
    primary_header->entry_size = GPT_ENTRY_SIZE;

    primary_header->entries_crc32 = 0;

    primary_header->header_crc32 = 0;    // Placeholder, should be calculated over actual entries


    return primary_header;
}

static GPTHeader *create_backup_gpt_header(int disk_no, uint64_t total_sectors){
    GPTHeader *backup_header = malloc(sizeof(GPTHeader));
    memset(backup_header, 0, sizeof(GPTHeader));

    // GPT Header fields
    memcpy(backup_header->signature, "EFI PART", 8);
    backup_header->revision = 0x00010000;  // Version 1.0
    backup_header->header_size = sizeof(GPTHeader);

    // Calculate CRC32 of the header (excluding CRC32 field itself)
    backup_header->header_crc32 = 0; // Temporarily 0 for CRC calculation
    backup_header->reserved = 0;

    backup_header->current_lba = total_sectors - 1;  // Backup GPT header at last LBA
    backup_header->backup_lba = 1;  // Primary GPT header is at LBA 1
    
    uint64_t entry_sectors = (int) (GPT_ENTRIES_COUNT * sizeof(GPTPartitionEntry) + SECTOR_SIZE - 1) / SECTOR_SIZE; // Should be 32 sectors for 128 entries of 128 bytes each

    backup_header->first_usable_lba = entry_sectors + 2;                // mbr(0) + primary gpt(1) + partition entries(2-33)
    backup_header->last_usable_lba = total_sectors - entry_sectors - 2;      // before backup entries and backup gpt

    memcpy(backup_header->disk_guid, DISK_GUID_EXAMPLE, 16); // Example GUID
    backup_header->entries_lba = total_sectors - entry_sectors - 1;
    backup_header->entries_count = GPT_ENTRIES_COUNT;
    backup_header->entry_size = GPT_ENTRY_SIZE;

    backup_header->entries_crc32 = 0; // Placeholder, should be calculated over actual entries

    backup_header->header_crc32 = 0; // Calculate CRC32 of the header (excluding CRC32 field itself);

    return backup_header;
}


bool create_esp_and_data_partitions(int disk_no, uint64_t esp_partition_sectors, uint64_t data_partition_sectors, uint64_t total_sectors) {
    
    // Write Protective MBR (Fixed)
    ProtectiveMBR mbr;
    create_protective_mbr(&mbr, total_sectors);
    if (!kebla_disk_write(disk_no, 0, 1, &mbr)) {
        return false;
    }

    // Creating Partition Entries
    uint8_t esp_guid[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00};    // Just an example GUID
    uint64_t esp_start_lba = ESP_START_LBA;  // 2048
    uint64_t esp_end_lba = esp_start_lba + esp_partition_sectors - 1;
    GPTPartitionEntry esp_entry = create_partition_entry(ESP_TYPE_GUID, esp_guid, esp_start_lba, esp_end_lba, 1ULL << 0, "EFI System Partition");

    uint8_t data_guid[16] = {0x00, 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11};   // Just an example GUID
    uint64_t data_start_lba = esp_end_lba + 1;
    uint64_t entry_sectors = (GPT_ENTRIES_COUNT * GPT_ENTRY_SIZE + SECTOR_SIZE - 1) / SECTOR_SIZE;
    uint64_t last_usable_lba = total_sectors - entry_sectors - 2;
    uint64_t data_end_lba = last_usable_lba;
    GPTPartitionEntry data_entry = create_partition_entry(LINUX_FS_GUID, data_guid, data_start_lba, data_end_lba, 0, "Kebla OS Root");


    // Create partition entries array
    GPTPartitionEntry partitions[GPT_ENTRIES_COUNT];
    memset(partitions, 0, sizeof(partitions));


    partitions[0] = esp_entry;
    partitions[1] = data_entry;
    
    int total_entries_sectors = (int) (GPT_ENTRIES_COUNT * GPT_ENTRY_SIZE + SECTOR_SIZE - 1) / SECTOR_SIZE;

    // Write primary partition entries
    if(!kebla_disk_write(disk_no, GPT_ENTRIES_LBA, total_entries_sectors, partitions)) {
        return false;
    }

    // Write backup partition entries
    uint64_t backup_entries_lba = (total_sectors - total_entries_sectors - 1);
    if (!kebla_disk_write(disk_no, backup_entries_lba, total_entries_sectors, partitions)) {
        return false;
    }

    uint32_t entries_crc = crc32(partitions, GPT_ENTRIES_COUNT * GPT_ENTRY_SIZE);

    // Create and write PRIMARY and BACKUP GPT Header
    GPTHeader *primary_header = create_primary_gpt_header(disk_no, total_sectors);
    
    if (!primary_header) {
        return false;
    }
    primary_header->entries_crc32 = entries_crc;
    primary_header->header_crc32 = crc32(primary_header, primary_header->header_size);

    // Write primary GPT header
    if (!kebla_disk_write(disk_no, 1, 1, primary_header)) {
        return false;
    }

    GPTHeader *backup_header = create_backup_gpt_header(disk_no, total_sectors);
    if (!backup_header) {
        return false;
    }
    backup_header->entries_crc32 = entries_crc;
    backup_header->header_crc32 = crc32(backup_header, backup_header->header_size);

    // Write backup GPT header
    if (!kebla_disk_write(disk_no, total_sectors - 1, 1, backup_header)) {
        return false;
    }

    free(primary_header);
    free(backup_header);

    return true;
}











