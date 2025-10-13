
#include "fatfs_wrapper.h"

#include "../driver/disk/disk.h"

#include "../lib/stdio.h"
#include "../lib/stdlib.h"
#include "../lib/string.h"

#include "iso9660.h"




// Utility functions for endian conversion
static uint32_t read_u32_le(const uint8_t *data) {
    return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}

static uint32_t read_u32_be(const uint8_t *data) {
    return data[3] | (data[2] << 8) | (data[1] << 16) | (data[0] << 24);
}

static uint16_t read_u16_le(const uint8_t *data) {
    return data[0] | (data[1] << 8);
}

// Read sectors from CD/DVD
static bool iso9660_read_sectors(int disk_no, uint32_t lba, void *buffer, uint32_t count) {
    return kebla_disk_read(disk_no, lba, count, buffer);
}


// Initialize ISO9660 filesystem
bool iso9660_init(int disk_no) {

    if(disk_no > disk_count - 1) return false;

    Disk disk = disks[disk_no];
    if(disk.context == 0 || disk.type != DISK_TYPE_SATAPI) return false;

    // Read Volume Descriptors (start at sector 16)
    uint8_t sector_buffer[2048];
    
    for (int i = 16; i < 32; i++) {
        if (!iso9660_read_sectors(disk_no, i, sector_buffer, 1)) {
            printf("ISO9660: Failed to read sector %d\n", i);
            return false;
        }
        
        uint8_t vd_type = sector_buffer[0];
        
        // Check for Primary Volume Descriptor
        if (vd_type == 0x01) {
            iso9660_pvd_t *pvd = (iso9660_pvd_t *)sector_buffer;
            
            // Verify identifier
            if (memcmp(pvd->identifier, "CD001", 5) != 0) {
                printf("ISO9660: Invalid volume descriptor identifier\n");
                return false;
            }
       
            disks[disk_no].bytes_per_sector = read_u16_le((uint8_t*)&pvd->logical_block_size_le);
            disks[disk_no].total_sectors = read_u32_le((uint8_t*)&pvd->volume_space_size_le);

            disks[disk_no].root_directory_sector = read_u32_le((uint8_t*)&pvd->root_directory_record.extent_location_le);
            disks[disk_no].root_directory_size = read_u32_le((uint8_t*)&pvd->root_directory_record.data_length_le);
            disks[disk_no].pvd_sector = i;
            
            printf("ISO9660: Found filesystem in disk-%d\n", disk_no);
            printf("  Volume ID: %s\n", pvd->volume_id);
            printf("  System ID: %s\n", pvd->system_id);
            printf("  Sector Size: %u\n",  disks[disk_no].bytes_per_sector);
            printf("  Total Sectors: %d\n", disks[disk_no].total_sectors);
            printf("  Root Directory: sector=%u, size=%u\n",  disks[disk_no].root_directory_sector, disks[disk_no].root_directory_size);
            
            return true;
        }
        
        // Stop at Volume Descriptor Set Terminator
        if (vd_type == 0xFF) {
            break;
        }
    }
    
    printf("ISO9660: No valid Primary Volume Descriptor found\n");

    return false;
}


// Parse directory record filename
static void parse_filename(const iso9660_dir_record_t *record, char *output, size_t output_size) {
    uint8_t name_len = record->file_id_length;
    
    // Handle version suffix (e.g., ";1")
    if (name_len >= 2 && record->file_id[name_len - 2] == ';') {
        name_len -= 2;
    }
    
    // Copy filename
    size_t copy_len = (name_len < output_size - 1) ? name_len : output_size - 1;
    memcpy(output, record->file_id, copy_len);
    output[copy_len] = '\0';
    
    // Replace directory separator if present
    for (size_t i = 0; i < copy_len; i++) {
        if (output[i] == '\0' || output[i] == '\x01') {
            output[i] = '_';
        }
    }
}



// List directory contents
bool iso9660_list_directory(int disk_no, uint32_t directory_sector, uint32_t directory_size) {
    if(!disks){
        printf("disks: %x\n", (uint64_t)disks);
        return false;
    }

    Disk disk= disks[disk_no];

    if (!disk.context || disk.bytes_per_sector == 0 || disk.total_sectors == 0 || disk.type != DISK_TYPE_SATAPI){
        printf("\ndisk variable not initialized properly!\n");
        printf("    context: %x\n", disk.context);
        printf("    bytes per sector: %d\n", disk.bytes_per_sector);
        printf("    total sectors: %d\n", disk.total_sectors);
        printf("    type: %d\n", disk.type);
        
        return false;
    } 
    
    uint8_t *buffer = malloc(directory_size);
    if (!buffer) {
        printf("Memory allocation failed!\n");
        return false;
    }
    
    // Calculate number of sectors to read
    uint32_t sector_count = (directory_size + disk.bytes_per_sector - 1) / disk.bytes_per_sector;
    
    if (!kebla_disk_read(disk_no, directory_sector, sector_count, buffer)) {
        printf("ISO9660: Failed to read directory\n");
        free(buffer);
        return false;
    }
    
    printf("Directory listing:\n");
    printf("SIZE    SECTOR  TYPE    NAME\n");
    printf("------  ------  ------  ----\n");
    
    uint8_t *ptr = buffer;
    uint32_t processed = 0;
    
    while (processed < directory_size) {
        iso9660_dir_record_t *record = (iso9660_dir_record_t *)ptr;
        
        // Check for end of records
        if (record->length == 0) {
            ptr += disk.bytes_per_sector - (processed % disk.bytes_per_sector);
            processed += disk.bytes_per_sector - (processed % disk.bytes_per_sector);
            continue;
        }
        
        // Skip current and parent directory entries
        if (record->file_id_length == 1 && 
            (record->file_id[0] == 0x00 || record->file_id[0] == 0x01)) {
            ptr += record->length;
            processed += record->length;
            continue;
        }
        
        // Get file info
        char filename[256];
        parse_filename(record, filename, sizeof(filename));
        
        uint32_t file_sector = read_u32_le((uint8_t*)&record->extent_location_le);
        uint32_t file_size = read_u32_le((uint8_t*)&record->data_length_le);
        bool is_directory = (record->file_flags & 0x02) != 0;
        
        printf("%u  %u  %s  %s\n", file_size, file_sector, is_directory ? "DIR" : "FILE", filename);
        
        ptr += record->length;
        processed += record->length;
    }
    
    free(buffer);
    return true;
}


// Read file from ISO9660
bool iso9660_read_file(int disk_no, uint32_t file_sector, uint32_t file_size, void *buffer) {
    if((disk_no > disk_count - 1) || !disks) return false;

    Disk disk = disks[disk_no];
    if(disk.context == 0 || disk.type != DISK_TYPE_SATAPI) return false;

    
    // Calculate number of sectors to read
    uint32_t sector_count = (file_size + disk.bytes_per_sector - 1) / disk.bytes_per_sector;
    
    if (!kebla_disk_read(disk_no, file_sector, sector_count, buffer)) {
        printf("ISO9660: Failed to read file data\n");
        return false;
    }
    
    return true;
}


// Find file in directory
bool iso9660_find_file(int disk_no, uint32_t directory_sector, uint32_t directory_size, const char *filename, iso9660_file_t *result) {
    if((disk_no > disk_count - 1) || !disks) return false;

    Disk disk = disks[disk_no];
    if(disk.context == 0 || disk.type != DISK_TYPE_SATAPI) return false;
    
    uint8_t *buffer = malloc(directory_size);
    if (!buffer) return false;
    
    uint32_t sector_count = (directory_size + disk.bytes_per_sector - 1) / disk.bytes_per_sector;
    
    if (!kebla_disk_read(disk_no, directory_sector, sector_count, buffer)) {
        free(buffer);
        return false;
    }
    
    uint8_t *ptr = buffer;
    uint32_t processed = 0;
    bool found = false;
    
    while (processed < directory_size && !found) {
        iso9660_dir_record_t *record = (iso9660_dir_record_t *)ptr;
        
        if (record->length == 0) {
            ptr += disk.bytes_per_sector - (processed % disk.bytes_per_sector);
            processed += disk.bytes_per_sector - (processed % disk.bytes_per_sector);
            continue;
        }
        
        // Skip current and parent directory
        if (record->file_id_length == 1 && 
            (record->file_id[0] == 0x00 || record->file_id[0] == 0x01)) {
            ptr += record->length;
            processed += record->length;
            continue;
        }
        
        char current_filename[256];
        parse_filename(record, current_filename, sizeof(current_filename));
        
        if (strcmp(current_filename, filename) == 0) {
            // Found the file
            strcpy(result->name, current_filename);
            result->sector = read_u32_le((uint8_t*)&record->extent_location_le);
            result->size = read_u32_le((uint8_t*)&record->data_length_le);
            result->is_dir = (record->file_flags & 0x02) != 0;
            found = true;
        }
        
        ptr += record->length;
        processed += record->length;
    }
    
    free(buffer);

    return found;
}

// Copy file from CD/DVD to memory
bool iso9660_copy_file(int disk_no, const char *filepath, void **buffer, uint32_t *size) {
    if((disk_no > disk_count - 1) || !disks) return false;

    Disk disk = disks[disk_no];
    if(disk.context == 0 || disk.type != DISK_TYPE_SATAPI) return false;
    
    // Start from root directory
    uint32_t current_sector = disk.root_directory_sector;
    uint32_t current_size = disk.root_directory_size;
    
    // Handle path traversal (simple implementation)
    char path[256];
    strcpy(path, filepath);
    
    char *token = strtok(path, "/");
    iso9660_file_t file_info;
    
    while (token != NULL) {
        if (!iso9660_find_file(disk_no, current_sector, current_size, token, &file_info)) {
            printf("ISO9660: File not found: %s\n", token);
            return false;
        }
        
        if (file_info.is_dir) {
            // Continue traversing
            current_sector = file_info.sector;
            current_size = file_info.size;
        } else {
            // Found the file
            *size = file_info.size;
            *buffer = malloc(file_info.size);
            
            if (!*buffer) {
                printf("ISO9660: Failed to allocate memory for file\n");
                return false;
            }
            
            uint32_t sector_count = (file_info.size + disk.bytes_per_sector - 1) / disk.bytes_per_sector;

            if (!kebla_disk_read(disk_no, file_info.sector, sector_count, *buffer)) {
                free(*buffer);
                *buffer = NULL;
                return false;
            }
            
            printf("ISO9660: Successfully copied file '%s' (%u bytes)\n", filepath, file_info.size);
            return true;
        }
        
        token = strtok(NULL, "/");
    }
    
    printf("ISO9660: Path leads to directory, not file\n");
    return false;
}



void iso9660_disk_test(int disk_no) {

    if((disk_no > disk_count - 1) || !disks) {
        printf("disk_count: %d, disks: %x\n", disk_count, (uint64_t)disks);
        return;
    }

    Disk disk = disks[disk_no];
    if(disk.context == 0 || disk.type != DISK_TYPE_SATAPI) {
        printf("disks[%d].context: %x, disk.type: %d\n", disk_no, disk.context, disk.type);
        return;
    }
    
    printf("Initializing ISO9660 filesystem...\n");
    if (!iso9660_init(disk_no)) {
        printf("Failed to initialize ISO9660 filesystem\n");
        return;
    }
    
    // List root directory
    printf("\nRoot directory contents:\n");
    if(iso9660_list_directory(disk_no, disks[disk_no].root_directory_sector, disks[disk_no].root_directory_size)){
        printf("Success listing root directory of iso9660 FS of Disk %d\n\n", disk_no);
    }else{
        printf("Failed listing root directory of iso9660 FS of Disk %d\n\n", disk_no);
    }

    iso9660_print_all_files(disk_no);
    
    // Example: Copy a specific file
    void *file_data = NULL;
    uint32_t file_size = 0;
    
    if (iso9660_copy_file(disk_no, "BOOT/USER_MAI.ELF", &file_data, &file_size)){
        printf("Successfully read BOOT/USER_MAI.ELF (%u bytes)\n", file_size);
        
        // Print first 100 bytes as example
        printf("First 100 bytes:\n");
        for (int i = 0; i < 100 && i < file_size; i++) {
            printf("%c", ((char*)file_data)[i]);
        }
        printf("\n");
        
        free(file_data);
    }

}











// Recursive function to traverse directory tree and print all files
static void iso9660_traverse_and_print(int disk_no, uint32_t directory_sector, 
                                      uint32_t directory_size, const char *current_path) {
    if (!disks || disk_no > disk_count - 1) return;
    
    Disk disk = disks[disk_no];
    if (!disk.context || disk.type != DISK_TYPE_SATAPI) return;
    
    // Read directory contents
    uint8_t *buffer = malloc(directory_size);
    if (!buffer) return;
    
    uint32_t sector_count = (directory_size + disk.bytes_per_sector - 1) / disk.bytes_per_sector;
    
    if (!kebla_disk_read(disk_no, directory_sector, sector_count, buffer)) {
        free(buffer);
        return;
    }
    
    uint8_t *ptr = buffer;
    uint32_t processed = 0;
    
    while (processed < directory_size) {
        iso9660_dir_record_t *record = (iso9660_dir_record_t *)ptr;
        
        // Check for end of records
        if (record->length == 0) {
            ptr += disk.bytes_per_sector - (processed % disk.bytes_per_sector);
            processed += disk.bytes_per_sector - (processed % disk.bytes_per_sector);
            continue;
        }
        
        // Skip current (0x00) and parent (0x01) directory entries
        if (record->file_id_length == 1 && 
            (record->file_id[0] == 0x00 || record->file_id[0] == 0x01)) {
            ptr += record->length;
            processed += record->length;
            continue;
        }
        
        // Parse filename
        char filename[256];
        parse_filename(record, filename, sizeof(filename));
        
        // Get file info
        uint32_t file_sector = read_u32_le((uint8_t*)&record->extent_location_le);
        uint32_t file_size = read_u32_le((uint8_t*)&record->data_length_le);
        bool is_directory = (record->file_flags & 0x02) != 0;
        
        // Build full path
        char full_path[512];
        if (strcmp(current_path, "/") == 0) {
            snprintf(full_path, sizeof(full_path), "/%s", filename);
        } else {
            snprintf(full_path, sizeof(full_path), "%s/%s", current_path, filename);
        }
        
        // Print file/directory info
        printf("%x %x %s %s\n", file_size, file_sector, is_directory ? "DIR" : "FILE", full_path);
        
        // If it's a directory, recursively traverse it
        if (is_directory) {
            iso9660_traverse_and_print(disk_no, file_sector, file_size, full_path);
        }
        
        ptr += record->length;
        processed += record->length;
    }
    
    free(buffer);
}

// Main function to print all files and directories in ISO9660 filesystem
void iso9660_print_all_files(int disk_no) {
    
    printf("\nComplete ISO9660 Filesystem Listing:\n");
    printf("SIZE     SECTOR   TYPE     PATH\n");
    printf("-------- -------- -------- ----\n");
    
    // Start recursive traversal from root directory
    iso9660_traverse_and_print(disk_no, 
                              disks[disk_no].root_directory_sector, 
                              disks[disk_no].root_directory_size, 
                              "/");
    
    printf("\nFile listing completed.\n");
}

// Alternative function that collects all files into an array (if you need to process them later)
typedef struct {
    char path[512];
    uint32_t sector;
    uint32_t size;
    bool is_dir;
} iso9660_file_entry_t;

static void iso9660_collect_files_recursive(int disk_no, uint32_t directory_sector, 
                                           uint32_t directory_size, const char *current_path,
                                           iso9660_file_entry_t **entries, int *count, int *capacity) {
    if (!disks || disk_no > disk_count - 1) return;
    
    Disk disk = disks[disk_no];
    if (!disk.context || disk.type != DISK_TYPE_SATAPI) return;
    
    // Read directory contents
    uint8_t *buffer = malloc(directory_size);
    if (!buffer) return;
    
    uint32_t sector_count = (directory_size + disk.bytes_per_sector - 1) / disk.bytes_per_sector;
    
    if (!kebla_disk_read(disk_no, directory_sector, sector_count, buffer)) {
        free(buffer);
        return;
    }
    
    uint8_t *ptr = buffer;
    uint32_t processed = 0;
    
    while (processed < directory_size) {
        iso9660_dir_record_t *record = (iso9660_dir_record_t *)ptr;
        
        if (record->length == 0) {
            ptr += disk.bytes_per_sector - (processed % disk.bytes_per_sector);
            processed += disk.bytes_per_sector - (processed % disk.bytes_per_sector);
            continue;
        }
        
        // Skip current and parent directory entries
        if (record->file_id_length == 1 && 
            (record->file_id[0] == 0x00 || record->file_id[0] == 0x01)) {
            ptr += record->length;
            processed += record->length;
            continue;
        }
        
        // Parse filename
        char filename[256];
        parse_filename(record, filename, sizeof(filename));
        
        // Build full path
        char full_path[512];
        if (strcmp(current_path, "/") == 0) {
            snprintf(full_path, sizeof(full_path), "/%s", filename);
        } else {
            snprintf(full_path, sizeof(full_path), "%s/%s", current_path, filename);
        }
        
        // Add to entries array (resize if needed)
        if (*count >= *capacity) {
            *capacity *= 2;
            *entries = realloc(*entries, *capacity * sizeof(iso9660_file_entry_t));
        }
        
        if (*entries) {
            iso9660_file_entry_t *entry = &(*entries)[*count];
            strcpy(entry->path, full_path);
            entry->sector = read_u32_le((uint8_t*)&record->extent_location_le);
            entry->size = read_u32_le((uint8_t*)&record->data_length_le);
            entry->is_dir = (record->file_flags & 0x02) != 0;
            (*count)++;
        }
        
        // If it's a directory, recursively traverse it
        if ((record->file_flags & 0x02) != 0) {
            iso9660_collect_files_recursive(disk_no, 
                                          read_u32_le((uint8_t*)&record->extent_location_le),
                                          read_u32_le((uint8_t*)&record->data_length_le),
                                          full_path, entries, count, capacity);
        }
        
        ptr += record->length;
        processed += record->length;
    }
    
    free(buffer);
}

// Function to print all files from collected array
void iso9660_print_all_files_collected(int disk_no) {
    if ((disk_no > disk_count - 1) || !disks) {
        printf("Invalid disk number or disks not initialized\n");
        return;
    }

    Disk disk = disks[disk_no];
    if (disk.context == 0 || disk.type != DISK_TYPE_SATAPI) {
        printf("Disk %d is not a valid SATAPI device\n", disk_no);
        return;
    }
    
    printf("Initializing ISO9660 filesystem...\n");
    if (!iso9660_init(disk_no)) {
        printf("Failed to initialize ISO9660 filesystem\n");
        return;
    }
    
    // Collect all files
    int capacity = 100;
    int count = 0;
    iso9660_file_entry_t *entries = malloc(capacity * sizeof(iso9660_file_entry_t));
    
    if (!entries) {
        printf("Memory allocation failed\n");
        return;
    }
    
    iso9660_collect_files_recursive(disk_no, 
                                   disks[disk_no].root_directory_sector,
                                   disks[disk_no].root_directory_size,
                                   "/", &entries, &count, &capacity);
    
    // Print collected files
    printf("\nComplete ISO9660 Filesystem Listing (%d entries):\n", count);
    printf("SIZE     SECTOR   TYPE     PATH\n");
    printf("-------- -------- -------- ----\n");
    
    for (int i = 0; i < count; i++) {
        printf("%u %u %s %s\n", 
               entries[i].size, 
               entries[i].sector,
               entries[i].is_dir ? "DIR" : "FILE",
               entries[i].path);
    }
    
    free(entries);
    printf("\nFile listing completed.\n");
}




