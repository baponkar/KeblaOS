#include "../lib/stdio.h"
#include "../driver/disk/disk.h"
#include "gpt_header.h"



// ===================== CRC32 ============================================================================
static uint32_t crc32_table[256];
static int crc32_table_computed = 0;

static void generate_crc32_table(void) {
    if (crc32_table_computed) return;
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++)
            c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
        crc32_table[i] = c;
    }
    crc32_table_computed = 1;
}

static uint32_t crc32(const void *buf, size_t len) {
    generate_crc32_table();
    const uint8_t *data = (const uint8_t *)buf;
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++)
        crc = (crc >> 8) ^ crc32_table[(crc ^ data[i]) & 0xFF];
    return ~crc;
}

// ===================== PROTECTIVE MBR ==================================================================
bool create_protective_mbr(int disk_no) {
    if (!disks || disk_no >= disk_count) return false;

    Disk disk = disks[disk_no];
    uint64_t total_sectors = disk.total_sectors;

    protective_mbr_t pmbr = {0};
    pmbr.partitions[0].boot_indicator = 0x00;
    pmbr.partitions[0].partition_type = 0xEE;           // Protective MBR
    pmbr.partitions[0].starting_lba = 1;
    pmbr.partitions[0].size_in_lba = (total_sectors > 0xFFFFFFFFULL) ? 0xFFFFFFFF : (uint32_t)(total_sectors - 1);
    pmbr.signature = 0xAA55;

    return kebla_disk_write(disk_no, 0, 1, &pmbr);      // Protective MBR at Sector 0
}


// ===================== GPT CREATION =====================================================================
bool create_gpt_header(int disk_no, uint32_t partition_crc) {
    if (!disks || disk_no >= disk_count) return false;

    Disk disk = disks[disk_no];
    uint64_t total_sectors = disk.total_sectors;

    gpt_header_t header = {0};
    header.signature = 0x5452415020494645ULL;  // "EFI PART"
    header.revision = 0x00010000;
    header.header_size = 92;
    header.current_lba = 1;
    header.backup_lba = total_sectors - 1;
    header.first_usable_lba = 34;
    header.last_usable_lba = total_sectors - 34;
    header.partition_entries_lba = 2;
    header.num_partition_entries = 128;
    header.partition_entry_size = sizeof(gpt_partition_entry_t);
    header.partition_entries_crc32 = partition_crc;

    for (int i = 0; i < 16; i++)
        header.disk_guid[i] = 0x11 + i;

    header.header_crc32 = 0;
    header.header_crc32 = crc32(&header, header.header_size);

    bool success = kebla_disk_write(disk_no, 1, 1, &header);    // Primary GPT Header at Sector 1

    // Backup GPT Header
    header.current_lba = total_sectors - 1;
    header.backup_lba = 1;
    header.partition_entries_lba = total_sectors - 33;
    header.header_crc32 = 0;
    header.header_crc32 = crc32(&header, header.header_size);

    success = success && kebla_disk_write(disk_no, total_sectors - 1, 1, &header); // Backup Primary Header at sector n-1
    
    return success;
}

bool create_partition_entries(int disk_no, uint32_t *out_crc) {
    if (!disks || disk_no >= disk_count) return false;

    Disk disk = disks[disk_no];
    uint64_t total_sectors = disk.total_sectors;
    gpt_partition_entry_t partitions[128] = {0};

    // EFI System Partition Type GUID
    uint8_t efi_type_guid[16] = {
        0x28, 0x73, 0x2A, 0xC1,
        0x1F, 0xF8,
        0xD2, 0x11,
        0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B
    };

    memcpy(partitions[0].partition_type_guid, efi_type_guid, 16);
    for (int i = 0; i < 16; i++){
        partitions[0].unique_partition_guid[i] = 0x22 + i;
    }

    partitions[0].starting_lba = 2048;
    partitions[0].ending_lba = total_sectors - 34;
    partitions[0].attributes = 0x8000000000000001ULL;

    const char *name = "EFI System Partition";
    for (int i = 0; i < 36 && name[i]; i++) {
        partitions[0].partition_name[i] = name[i];
    }

    uint32_t entry_crc = crc32(partitions, sizeof(partitions));
    if (out_crc) *out_crc = entry_crc;

    bool success = kebla_disk_write(disk_no, 2, 32, partitions);             // Primary Partition Entries at Sector 2-32
    uint64_t lba = total_sectors - 33;
    printf(" Total Sector: %d ,LBA %d\n", total_sectors, lba);
    success = success && kebla_disk_write(disk_no, lba, 32, partitions);     // Backup Primary Entries n-33  to n-2
    return success;
}


// ===================== COMPLETE GPT CREATION ============================================================
bool create_complete_gpt(int disk_no) {
    uint32_t partition_crc = 0;

    if (!create_protective_mbr(disk_no)){
        printf(" Failed to create protective mbr in Disk %d\n", disk_no);
        return false;
    }

    if (!create_partition_entries(disk_no, &partition_crc)){
        printf(" Failed to write partition entriies in Disk %d\n", disk_no);
        return false;
    }

    if (!create_gpt_header(disk_no, partition_crc)){
        printf(" Failed to write GPT Header in Disk %d\n", disk_no);
        return false;
    }

    return true;
}





















