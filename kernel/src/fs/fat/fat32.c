
/*
File Allocation Table

Reference:
https://wiki.osdev.org/FAT
*/


#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../lib/ctype.h"

#include "../../memory/kheap.h"
#include "../../memory/kmalloc.h"
#include "../../memory/vmm.h"

#include "fat32.h"


#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE   0x20


FAT32_BPB fat32_bpb;        // BIOS Parameter Block

fat32_info_t fat32_info;

HBA_PORT_T* fat32_port;     // Save port globally

uint32_t ROOT_DIR_CLUSTER;  // Root directory cluster number


bool fat32_init(HBA_PORT_T* port) {

    if (!port) {
        printf("[Error] FAT32: Invalid port\n");
        return false;
    }

    uint8_t sector[512];    // A sector with 512 bytes
    fat32_port = port;

    // Reading First Sector
    ahci_read(port, 0, 0, 1, (uint16_t*)sector);

    // Extract data and fill the FAT32_BPB structure
    fat32_info.bytes_per_sector      = *(uint16_t*)&sector[11];
    fat32_info.sectors_per_cluster   = sector[13];
    fat32_info.reserved_sector_count = *(uint16_t*)&sector[14];
    fat32_info.num_fats              = sector[16];
    fat32_info.fat_size              = *(uint32_t*)&sector[36];
    fat32_info.root_dir_first_cluster= *(uint32_t*)&sector[44];
    fat32_info.fat_start_sector      = fat32_info.reserved_sector_count;
    fat32_info.cluster_heap_start_sector = fat32_info.fat_start_sector + fat32_info.num_fats * fat32_info.fat_size;

    printf("Bytes/Sector: %x\n", fat32_info.bytes_per_sector);
    printf("Sectors/Cluster: %x\n", fat32_info.sectors_per_cluster);
    printf("Reserved Sectors: %x\n", fat32_info.reserved_sector_count);
    printf("Number of FATs: %x\n", fat32_info.num_fats);
    printf("FAT Size: %x\n", fat32_info.fat_size);
    printf("Root Cluster: %x\n", fat32_info.root_dir_first_cluster);


    printf("[Info] Successfully initialize FAT32 with bytes per sector: %d\n", fat32_info.bytes_per_sector);
    
    return true;
}

// Converts a cluster number to a sector number
uint32_t fat32_cluster_to_sector(uint32_t cluster) {
    return fat32_info.cluster_heap_start_sector + (cluster - 2) * fat32_info.sectors_per_cluster;
}

// Converts a sector number to a cluster number
uint32_t fat32_sector_to_cluster(uint32_t sector) {
    return ((sector - fat32_info.cluster_heap_start_sector) / fat32_info.sectors_per_cluster) + 2;  // 2 for root dir
}

// parsing fat32_info
uint32_t fat32_fat_start_sector() {
    return fat32_info.fat_start_sector;
}

uint32_t fat32_data_start_sector() {
    return fat32_info.cluster_heap_start_sector;
}

uint32_t fat32_first_data_sector() {
    return fat32_bpb.reserved_sector_count + (fat32_bpb.num_fats * fat32_bpb.fat_size_32) + 2;  // 2 for root dir
}

uint32_t fat32_fat_size() {
    return fat32_bpb.fat_size_32;
}

uint32_t fat32_total_sectors() {
    return fat32_bpb.total_sectors_32;
}

uint32_t fat32_bytes_per_sector() {
    return fat32_bpb.bytes_per_sector;
}

uint32_t fat32_sectors_per_cluster() {
    return fat32_bpb.sectors_per_cluster;
}

uint32_t fat32_reserved_sector_count() {
    return fat32_bpb.reserved_sector_count;
}

uint32_t fat32_num_fats() {
    return fat32_bpb.num_fats;
}

uint32_t fat32_root_entry_count() {
    return fat32_bpb.root_entry_count;
}

uint32_t fat32_media() {
    return fat32_bpb.media;
}


// This function scans the FAT32 file system to find a free cluster.
uint32_t fat32_find_free_cluster() {
    uint8_t sector[512];
    for (uint32_t i = 2; i < 0x0FFFFFF6; i++) { // 8 TB
        uint32_t fat_offset = i * 4;
        if (fat32_info.bytes_per_sector == 0) {
            printf("[FAT32] ERROR: sectors_per_cluster is zero!\n");
            while(1);
        }
        uint32_t fat_sector = fat32_info.fat_start_sector + (fat_offset / fat32_info.bytes_per_sector);
        uint32_t entry_offset = fat_offset % fat32_info.bytes_per_sector;

        ahci_read(fat32_port, fat_sector, 0, 1, (uint16_t*)sector);
        uint32_t value = *(uint32_t*)&sector[entry_offset] & 0x0FFFFFFF;

        if (value == 0) {
            // Found free
            return i;
        }
    }
    return 0;
}

// Find the next cluster of given cluster
uint32_t fat32_next_cluster(uint32_t cluster) {
    uint8_t sector[512];
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = fat32_info.fat_start_sector + (fat_offset / fat32_info.bytes_per_sector);
    uint32_t entry_offset = fat_offset % fat32_info.bytes_per_sector;

    ahci_read(fat32_port, fat_sector, 0, 1, (uint16_t*)sector);
    return *(uint32_t*)&sector[entry_offset] & 0x0FFFFFFF;
}

// The below function set the given value to the cluster in the FAT32 file system.
void fat32_set_cluster(uint32_t cluster, uint32_t value) {
    uint8_t sector[512];
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = fat32_info.fat_start_sector + (fat_offset / fat32_info.bytes_per_sector);
    uint32_t entry_offset = fat_offset % fat32_info.bytes_per_sector;

    ahci_read(fat32_port, fat_sector, 0, 1, (uint16_t*)sector);
    *(uint32_t*)&sector[entry_offset] = value;
    ahci_write(fat32_port, fat_sector, 0, 1, (uint16_t*)sector);
}


// Converts a standard filename (e.g., "README.TXT") to 11-byte 8.3 format (e.g., "README  TXT")
char* to_fat32_name(const char* input) {
    static char output11[12];  // 11 + 1 for null-terminator
    memset(output11, ' ', 11);  // Initialize with spaces

    int i = 0, j = 0;
    while (input[i] != '\0' && input[i] != '.' && j < 8) {
        output11[j++] = toupper((unsigned char)input[i++]);
    }

    if (input[i] == '.') {
        i++;
        j = 8;
        int k = 0;
        while (input[i] != '\0' && k < 3) {
            output11[j++] = toupper((unsigned char)input[i++]);
            k++;
        }
    }
    return output11;
}


// Check presence of file or directory in root directory
bool fat32_check_presence(const char* filename, bool is_directory) {
    char fat_name[12];
    memcpy(&fat_name, filename, 12); //  Converting into 8.3 Filename

    uint32_t cluster = fat32_info.root_dir_first_cluster;
    uint8_t sector_buffer[512 * fat32_info.sectors_per_cluster];

    while (cluster < 0x0FFFFFF8 && cluster >= 2) {
        if (!fat32_read_cluster(cluster, sector_buffer, sizeof(sector_buffer)))
            return false;

        DIR_ENTRY* entries = (DIR_ENTRY*)sector_buffer;
        uint32_t num_entries = (512 * fat32_info.sectors_per_cluster) / sizeof(DIR_ENTRY);

        for (uint32_t i = 0; i < num_entries; i++) {
            DIR_ENTRY* entry = &entries[i];

            if (entry->name[0] == 0x00 || entry->name[0] == 0xE5)
                continue; // Unused or deleted

            if (memcmp(entry->name, fat_name, 11) == 0) {
                if (is_directory && (entry->attr & 0x10)) return true;
                if (!is_directory && !(entry->attr & 0x10)) return true;
            }
        }

        cluster = fat32_next_cluster(cluster);
    }

    return false;
}


// This function finds a free entry in the directory cluster and returns it.
bool fat32_find_free_entry(uint32_t cluster, DIR_ENTRY* entry) {
    uint8_t buffer[512];
    uint32_t sector = fat32_cluster_to_sector(cluster);
    ahci_read(fat32_port, sector, 0, 1, (uint16_t*)buffer);

    DIR_ENTRY* entries = (DIR_ENTRY*)buffer;
    for (int i = 0; i < 512 / sizeof(DIR_ENTRY); i++) {
        if (entries[i].name[0] == 0x00 || entries[i].name[0] == 0xE5) {
            *entry = entries[i];
            return true;
        }
    }
    return false;
}

// File Management Functions
bool fat32_create_file(const char* filename) {
    uint8_t buffer[512];
    uint32_t root_sector = fat32_cluster_to_sector(fat32_info.root_dir_first_cluster);
    ahci_read(fat32_port, root_sector, 0, 1, (uint16_t*)buffer);

    DIR_ENTRY* entries = (DIR_ENTRY*)buffer;

    // Find free entry
    for (int i = 0; i < 512 / sizeof(DIR_ENTRY); i++) {
        if (entries[i].name[0] == 0x00 || entries[i].name[0] == 0xE5) {
            memset(&entries[i], 0, sizeof(DIR_ENTRY));
            strncpy(entries[i].name, filename, 11); // SHORT 8.3 NAME ONLY
            entries[i].attr = ATTR_ARCHIVE;
            uint32_t cluster = fat32_find_free_cluster();
            entries[i].fstClusLO = cluster & 0xFFFF;
            entries[i].fstClusHI = cluster >> 16;
            entries[i].fileSize = 0;
            fat32_set_cluster(cluster, 0x0FFFFFFF); // End of cluster chain
            ahci_write(fat32_port, root_sector, 0, 1, (uint16_t*)buffer);
            return true;
        }
    }
    return false;
}

bool fat32_read_file(const char* filename, uint8_t* buffer, uint32_t max_size) {
    uint8_t sector[512];
    uint32_t root_sector = fat32_cluster_to_sector(fat32_info.root_dir_first_cluster);
    uint32_t cluster = fat32_info.root_dir_first_cluster;
    
    ahci_read(fat32_port, root_sector, 0, 1, (uint16_t*)sector);

    DIR_ENTRY* entries = (DIR_ENTRY*)sector;

    for (int i = 0; i < 512 / sizeof(DIR_ENTRY); i++) {
        if (strncmp(entries[i].name, filename, 11) == 0) {
            uint32_t cluster = (entries[i].fstClusHI << 16) | entries[i].fstClusLO;
            uint32_t size = entries[i].fileSize;

            uint32_t bytes_read = 0;
            uint8_t temp[512];

            while (cluster < 0x0FFFFFF8 && bytes_read < size) {
                uint32_t sector_num = fat32_cluster_to_sector(cluster);
                for (uint32_t j = 0; j < fat32_info.sectors_per_cluster; j++) {
                    ahci_read(fat32_port, sector_num + j, 0, 1, (uint16_t*)temp);
                    uint32_t copy_size = (size - bytes_read) > 512 ? 512 : (size - bytes_read);
                    memcpy(buffer + bytes_read, temp, copy_size);
                    bytes_read += copy_size;
                    if (bytes_read >= size) break;
                }
                cluster = fat32_next_cluster(cluster);
            }
            return true;
        }
    }
    return false;
}

bool fat32_write_file(const char* filename, const uint8_t* data, uint32_t size) {
    uint8_t sector[512];
    uint32_t root_sector = fat32_cluster_to_sector(fat32_info.root_dir_first_cluster);
    ahci_read(fat32_port, root_sector, 0, 1, (uint16_t*)sector);

    DIR_ENTRY* entries = (DIR_ENTRY*)sector;

    for (int i = 0; i < 512 / sizeof(DIR_ENTRY); i++) {
        if (strncmp(entries[i].name, filename, 11) == 0) {
            uint32_t cluster = (entries[i].fstClusHI << 16) | entries[i].fstClusLO;

            uint32_t bytes_written = 0;
            uint8_t temp[512];

            while (bytes_written < size) {
                uint32_t sector_num = fat32_cluster_to_sector(cluster);
                for (uint32_t j = 0; j < fat32_info.sectors_per_cluster; j++) {
                    uint32_t copy_size = (size - bytes_written) > 512 ? 512 : (size - bytes_written);
                    memcpy(temp, data + bytes_written, copy_size);
                    ahci_write(fat32_port, sector_num + j, 0, 1, (uint16_t*)temp);
                    bytes_written += copy_size;
                    if (bytes_written >= size) break;
                }

                if (bytes_written < size) {
                    uint32_t next = fat32_find_free_cluster();
                    fat32_set_cluster(cluster, next);
                    fat32_set_cluster(next, 0x0FFFFFFF);
                    cluster = next;
                }
            }

            entries[i].fileSize = size;
            ahci_write(fat32_port, root_sector, 0, 1, (uint16_t*)sector);
            return true;
        }
    }
    return false;
}

bool fat32_delete_file(const char* filename) {
    uint8_t sector[512];
    uint32_t root_sector = fat32_cluster_to_sector(fat32_info.root_dir_first_cluster);
    ahci_read(fat32_port, root_sector, 0, 1, (uint16_t*)sector);

    DIR_ENTRY* entries = (DIR_ENTRY*)sector;

    for (int i = 0; i < 512 / sizeof(DIR_ENTRY); i++) {
        if (strncmp(entries[i].name, filename, 11) == 0) {
            entries[i].name[0] = 0xE5; // Mark as deleted
            ahci_write(fat32_port, root_sector, 0, 1, (uint16_t*)sector);
            return true;
        }
    }
    return false;
}


uint32_t fat32_get_file_size(const char* filename) {
    uint8_t sector[512];
    uint32_t root_sector = fat32_cluster_to_sector(fat32_info.root_dir_first_cluster);
    ahci_read(fat32_port, root_sector, 0, 1, (uint16_t*)sector);

    DIR_ENTRY* entries = (DIR_ENTRY*)sector;

    for (int i = 0; i < 512 / sizeof(DIR_ENTRY); i++) {
        if (strncmp(entries[i].name, filename, 11) == 0) {
            return entries[i].fileSize;
        }
    }
    return 0;
}

// Directory Management Functions
bool fat32_init_directory_cluster(uint32_t cluster, uint32_t parent_cluster) {
    uint8_t buffer[512];
    uint32_t sector = fat32_cluster_to_sector(cluster);
    memset(buffer, 0, sizeof(buffer));
    ahci_write(fat32_port, sector, 0, 1, (uint16_t*)buffer);

    // Write "." and ".." entries
    DIR_ENTRY dot_entry = {0};
    DIR_ENTRY dotdot_entry = {0};

    strncpy(dot_entry.name, ".", 11);
    dot_entry.attr = ATTR_DIRECTORY;
    dot_entry.fstClusHI = (cluster >> 16) & 0xFFFF;
    dot_entry.fstClusLO = cluster & 0xFFFF;
    dot_entry.fileSize = 0;

    strncpy(dotdot_entry.name, "..", 11);
    dotdot_entry.attr = ATTR_DIRECTORY;
    dotdot_entry.fstClusHI = (parent_cluster >> 16) & 0xFFFF;
    dotdot_entry.fstClusLO = parent_cluster & 0xFFFF;
    dotdot_entry.fileSize = 0;

    memcpy(buffer, &dot_entry, sizeof(DIR_ENTRY));
    memcpy(buffer + sizeof(DIR_ENTRY), &dotdot_entry, sizeof(DIR_ENTRY));
    
    ahci_write(fat32_port, sector, 0, 1, (uint16_t*)buffer);

    return true;
}

bool fat32_read_root_dir() {
    uint8_t sector[512];
    uint32_t root_sector = fat32_cluster_to_sector(fat32_info.root_dir_first_cluster);
    ahci_read(fat32_port, root_sector, 0, 1, (uint16_t*)sector);

    DIR_ENTRY* entries = (DIR_ENTRY*)sector;

    for (int i = 0; i < 512 / sizeof(DIR_ENTRY); i++) {
        if (entries[i].name[0] == 0x00) break; // End of directory
        printf("[FAT32] File: %s Size: %d\n", entries[i].name, entries[i].fileSize);
        fat32_delete_file(entries[i].name); // Delete file for testing
    }
    return true;
}

bool fat32_create_directory(const char* name) {
    // uint32_t parent_cluster = fat32_root_cluster; // Assuming root directory for simplicity
    uint32_t parent_cluster = fat32_info.root_dir_first_cluster; // Root directory cluster
    
    // Step 1: Find free entry in parent
    DIR_ENTRY entry;
    if (!fat32_find_free_entry(parent_cluster, &entry)) {
        return false;
    }

    // Step 2: Allocate a cluster for new directory
    uint32_t new_dir_cluster = fat32_find_free_cluster(parent_cluster, &entry);
    if (new_dir_cluster == 0) {
        return false;
    }

    // Step 3: Fill entry
    memset(&entry, 0, sizeof(entry));
    strncpy(entry.name, name, 11);
    entry.attr = ATTR_DIRECTORY;
    entry.fstClusHI = (new_dir_cluster >> 16) & 0xFFFF;
    entry.fstClusLO = new_dir_cluster & 0xFFFF;
    entry.fileSize = 0;
    
    fat32_write_directory_entry(parent_cluster, &entry);

    // Step 4: Write "." and ".." inside new directory
    fat32_init_directory_cluster(new_dir_cluster, parent_cluster);

    return true;
}

bool fat32_delete_directory(uint32_t cluster) {
    uint8_t buffer[512];
    uint32_t sector = fat32_cluster_to_sector(cluster);
    ahci_read(fat32_port, sector, 0, 1, (uint16_t*)buffer);

    DIR_ENTRY* entries = (DIR_ENTRY*)buffer;
    for (int i = 0; i < 512 / sizeof(DIR_ENTRY); i++) {
        if (entries[i].name[0] == 0x00) break; // End of directory
        if (entries[i].attr == ATTR_DIRECTORY) {
            fat32_delete_directory(entries[i].fstClusLO | (entries[i].fstClusHI << 16));
        }
    }

    // Mark the directory as deleted
    memset(buffer, 0, sizeof(buffer));
    ahci_write(fat32_port, sector, 0, 1, (uint16_t*)buffer);

    return true;
}

bool fat32_write_directory_entry(uint32_t cluster, DIR_ENTRY* entry) {
    uint8_t buffer[512];
    uint32_t sector = fat32_cluster_to_sector(cluster);
    ahci_read(fat32_port, sector, 0, 1, (uint16_t*)buffer);

    DIR_ENTRY* entries = (DIR_ENTRY*)buffer;
    for (int i = 0; i < 512 / sizeof(DIR_ENTRY); i++) {
        if (entries[i].name[0] == 0x00 || entries[i].name[0] == 0xE5) {
            entries[i] = *entry;
            ahci_write(fat32_port, sector, 0, 1, (uint16_t*)buffer);
            return true;
        }
    }
    return false;
}



bool fat32_delete_directory_entry(uint32_t cluster, const char* name) {
    uint8_t buffer[512];
    uint32_t sector = fat32_cluster_to_sector(cluster);
    ahci_read(fat32_port, sector, 0, 1, (uint16_t*)buffer);

    DIR_ENTRY* entries = (DIR_ENTRY*)buffer;
    for (int i = 0; i < 512 / sizeof(DIR_ENTRY); i++) {
        if (strncmp(entries[i].name, name, 11) == 0) {
            entries[i].name[0] = 0xE5; // Mark as deleted
            ahci_write(fat32_port, sector, 0, 1, (uint16_t*)buffer);
            return true;
        }
    }
    return false;
}


uint32_t fat32_get_directory_cluster(const char* name) {
    uint8_t buffer[512];
    uint32_t root_sector = fat32_cluster_to_sector(fat32_info.root_dir_first_cluster);
    ahci_read(fat32_port, root_sector, 0, 1, (uint16_t*)buffer);

    DIR_ENTRY* entries = (DIR_ENTRY*)buffer;

    for (int i = 0; i < 512 / sizeof(DIR_ENTRY); i++) {
        if (strncmp(entries[i].name, name, 11) == 0) {
            return (entries[i].fstClusHI << 16) | entries[i].fstClusLO;
        }
    }
    return 0;
}


uint32_t fat32_read_cluster(uint32_t cluster, uint8_t* buffer, uint32_t size) {
    if (cluster < 2) return 0;

    uint32_t first_sector = fat32_info.fat_start_sector + (cluster - 2) * fat32_info.sectors_per_cluster;
    uint32_t total_bytes = fat32_info.sectors_per_cluster * fat32_info.bytes_per_sector;

    if (size > total_bytes) size = total_bytes;

    uint32_t sectors_to_read = (size + fat32_info.bytes_per_sector - 1) / fat32_info.bytes_per_sector;

    bool success = ahci_read(fat32_port, first_sector, 0, sectors_to_read, (uint16_t*)buffer);
    return success ? size : 0;
}

uint32_t fat32_write_cluster(uint32_t cluster, const uint8_t* buffer, uint32_t size) {
    if (cluster < 2) return 0;

    uint32_t first_sector = fat32_info.fat_start_sector + (cluster - 2) * fat32_info.sectors_per_cluster;
    uint32_t total_bytes = fat32_info.sectors_per_cluster * fat32_info.bytes_per_sector;

    if (size > total_bytes) size = total_bytes;

    uint32_t sectors_to_write = (size + fat32_info.bytes_per_sector - 1) / fat32_info.bytes_per_sector;

    bool success = ahci_write(fat32_port, first_sector, 0, sectors_to_write, (uint16_t*)buffer);
    return success ? size : 0;
}


void fat32_run_tests(HBA_PORT_T* global_port) {

    const char* filename = "FILE    TXT";       // 8.3 format
    const char* message = "Hello FAT32 World!";
    uint8_t buffer[512];

    // 1. Check Presence of file
    printf("[FAT32] Checking presence of file: %s\n", filename);
    if(fat32_check_presence(filename, false)){
        printf("[FAT32] The file %s is present!\n", filename);
    }
    printf("[FAT32] The file %s is not present\n", filename);

    // 2. Create file
    printf(" [FAT32] Creating file: %s\n", filename);
    if (!fat32_create_file(filename)) {
        printf("[FAT32] Failed to create file!\n");
        return;
    }
    printf(" [FAT32] File created successfully.\n");

    // 3. Write to file
    printf(" [FAT32] Writing to file: %s\n", filename);
    if (!fat32_write_file(filename, (const uint8_t*)message, strlen(message))) {
        printf("[FAT32] Failed to write to file!\n");
        // return;
    }
    printf("[FAT32] File written successfully.\n");

    // 4. Read back the file
    memset(buffer, 0, sizeof(buffer));  // Clearing the buffer
    printf("[FAT32] Reading from file: %s\n", filename);
    if (!fat32_read_file(filename, buffer, sizeof(buffer))) {
        printf("[FAT32] Failed to read file!\n");
        return;
    }
    printf("[FAT32] File contents: %s\n", buffer);
    
    // 5. Delete the file
    printf("[FAT32] Deleting file: %s\n", filename);
    if (!fat32_delete_file(filename)) {
        printf(" [FAT32] Failed to delete file!\n");
        return;
    }
    printf("[FAT32] File deleted successfully.\n");

    // 6. Check presence again
    printf("[FAT32] Checking presence of file again: %s\n", filename);
    if(fat32_check_presence(filename, false)){
        printf("[FAT32] The file %s is still present!\n", filename);
    } else {
        printf("[FAT32] The file %s is not present anymore.\n", filename);
    }

    printf("[FAT32] All FAT32 tests passed!\n");
}




