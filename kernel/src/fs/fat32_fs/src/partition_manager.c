
#include "../../../lib/string.h"
#include "../../../lib/stdlib.h"
#include "../../../lib/stdio.h"
#include "../../../lib/ctype.h"

#include "../include/diskio.h"

#include "../include/guid.h"
#include "../include/mbr.h"
#include "../include/gpt.h"

#include "../include/partition_manager.h"



#define TOTAL_SECTORS 1 * 1024 * 1024 * 1024 / 512   // 1GB = 1 * 1024 * 1024 * 1024 Byte
#define MAX_PARTITIONS 4

PartitionEntry partitions[MAX_PARTITIONS];  // Array of PartitionEntry

size_t partition_count = 0;

bool create_partition(uint8_t pdrv_no, uint64_t start_lba, uint64_t sectors, guid_t partition_guid, guid_t partition_type_guid, char* name) {
    
    size_t partition_index = partition_count; // Get the next available partition index

    // Creating and Write a Protective MBR at Sector 0
    ProtectiveMBR *protective_mbr = malloc(SECTOR_SIZE);
    create_protective_mbr(protective_mbr, TOTAL_SECTORS);

    if(!fat32_disk_write( 0, 1, protective_mbr)){
        printf("[PARTITION] Failed to write Protective MBR for disk!\n");
        free(protective_mbr);
        return false;
    }

    // ==========================================================

    if (partition_count >= MAX_PARTITIONS) {
        printf("Maximum number of partitions reached.\n");
        return false;
    }

    PartitionEntry entry = partitions[partition_index];
    entry.pdrv_no = pdrv_no;
    entry.partition_no = partition_index;
    entry.start_lba = start_lba;
    entry.sectors = sectors;
    memcpy(&entry.partition_guid, partition_guid, sizeof(guid_t));
    memcpy(&entry.partition_type_guid, partition_type_guid, sizeof(guid_t));

    // ======================================================================
    // Creating GPTPrtition
    GPTPartitionEntry gpt_partitions[GPT_ENTRIES_COUNT];
    memset(gpt_partitions, 0, sizeof(gpt_partitions));

    // Read Previous partition entries from disk
    int total_entries_sectors = (int) (GPT_ENTRIES_COUNT * GPT_ENTRY_SIZE + SECTOR_SIZE - 1) / SECTOR_SIZE; // This should be 32 sectors for 128 entries of 128 bytes each
    if(!fat32_disk_read(GPT_ENTRIES_START_LBA, total_entries_sectors, gpt_partitions)) {
        printf("[PARTITION] Failed to read primary GPT partition entries for disk!\n");
        return false;
    }
    
    // Creating new GPT partition entry for this partition
    GPTPartitionEntry *gpt_entry = create_gpt_partition_entry(partition_type_guid, partition_guid, start_lba, start_lba + sectors - 1, 0, name);
    if(gpt_entry != NULL) {
        memcpy(&entry.gpt_entry, gpt_entry, sizeof(GPTPartitionEntry));
    } else {
        return false;
    }

    gpt_partitions[partition_index] = *gpt_entry; // include the new partition entry in the array for GPTPartitionEntry array

    // Creating GPT Headers and Partition Entries
    GPTHeader primary_header;
    GPTHeader backup_header;


    // write partition entries to disk (primary and backup)
    if(!fat32_disk_write( GPT_ENTRIES_START_LBA, total_entries_sectors, gpt_partitions)) {
        printf("[PARTITION] Failed to write primary GPT partition entries for disk!\n");
        return false;
    }
    
    uint64_t backup_entries_lba = TOTAL_SECTORS - total_entries_sectors - 1;
    if (!fat32_disk_write( backup_entries_lba, total_entries_sectors, gpt_partitions)) {
        printf("[PARTITION] Failed to write backup GPT partition entries for disk!\n");
        return false;
    }

    // Create and write GPT headers
    bool result = create_gpt_header(TOTAL_SECTORS, &primary_header, &backup_header, DISK_GUID_EXAMPLE, gpt_partitions);

    if(result) {
        printf("Created partition %d on drive %d: Start LBA: %lu, Sectors: %lu\n", 
               entry.partition_no, entry.pdrv_no, entry.start_lba, entry.sectors);
    } else {
        printf("Failed to create GPT headers for partition %d on drive %d.\n", 
               entry.partition_no, entry.pdrv_no);
        return false;
    }

    partition_count++; // Increment the partition count

    return true;
}




bool update_partition(
    size_t partition_index,
    uint64_t new_start_lba,
    uint64_t new_sectors,
    const char* new_name,
    uint64_t new_attributes)
{
    if (partition_index >= GPT_ENTRIES_COUNT) {
        printf("[PARTITION] Invalid partition index!\n");
        return false;
    }

    // 1️⃣ Read existing GPT partition entries
    GPTPartitionEntry gpt_partitions[GPT_ENTRIES_COUNT];
    memset(gpt_partitions, 0, sizeof(gpt_partitions));

    uint32_t total_entries_sectors =
        (sizeof(GPTPartitionEntry) * GPT_ENTRIES_COUNT + SECTOR_SIZE - 1)
        / SECTOR_SIZE;

    if (!fat32_disk_read(GPT_ENTRIES_START_LBA,
                   total_entries_sectors,
                   gpt_partitions)) {
        printf("[PARTITION] Failed to read GPT entries!\n");
        return false;
    }

    GPTPartitionEntry *entry = &gpt_partitions[partition_index];

    // Check if entry is actually used
    bool empty = true;
    for (int i = 0; i < 16; i++) {
        if (entry->type_guid[i] != 0) {
            empty = false;
            break;
        }
    }

    if (empty) {
        printf("[PARTITION] Partition slot is empty!\n");
        return false;
    }

    // 2️⃣ Update fields
    entry->start_lba = new_start_lba;
    entry->end_lba   = new_start_lba + new_sectors - 1;
    entry->attributes = new_attributes;

    // Clear old name
    memset(entry->name, 0, sizeof(entry->name));

    // Write new UTF-16LE name
    for (size_t i = 0; i < 36 && new_name[i] != '\0'; i++) {
        entry->name[i * 2] = new_name[i];
    }

    // 3️⃣ Rewrite PRIMARY partition entry array
    if (!fat32_disk_write(GPT_ENTRIES_START_LBA,
                    total_entries_sectors,
                    gpt_partitions)) {
        printf("[PARTITION] Failed to write primary GPT entries!\n");
        return false;
    }

    // 4️⃣ Rewrite BACKUP partition entry array
    uint64_t backup_entries_lba =
        TOTAL_SECTORS - total_entries_sectors - 1;

    if (!fat32_disk_write(backup_entries_lba,
                    total_entries_sectors,
                    gpt_partitions)) {
        printf("[PARTITION] Failed to write backup GPT entries!\n");
        return false;
    }

    // 5️⃣ Recreate GPT headers (recalculate CRC)
    GPTHeader primary_header;
    GPTHeader backup_header;

    if (!create_gpt_header(TOTAL_SECTORS,
                           &primary_header,
                           &backup_header,
                           DISK_GUID_EXAMPLE,
                           gpt_partitions)) {
        printf("[PARTITION] Failed to update GPT headers!\n");
        return false;
    }

    printf("[PARTITION] Partition %lu updated successfully.\n",
           partition_index);

    return true;
}