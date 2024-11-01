#include "file_system.h"


// Function to read the boot sector from the disk (assuming the disk is already initialized)
void read_boot_sector(fat_boot_sector_t *boot_sector) {
    // Read the first sector (sector 0)
    // Use your own low-level disk reading functions
    disk_read(0, 1, (uint8_t *)boot_sector, SECTOR_SIZE);
}


void mount_fat(fat_boot_sector_t *boot_sector) {
    // Verify the file system type by checking the fs_type field
    if (strncmp(boot_sector->fs_type, "FAT16", 5) == 0) {
        // Proceed with FAT16 mounting logic
        print("FAT16 file system detected.\n");
    } else if (strncmp(boot_sector->fs_type, "FAT32", 5) == 0) {
        // Proceed with FAT32 mounting logic
        print("FAT32 file system detected.\n");
    } else {
        print("Unknown or unsupported file system type.\n");
    }

    // Read and load the FAT into memory (you'll need to write a function for this)
    // E.g., read_fat(boot_sector->reserved_sectors + 1, boot_sector->num_fats, fat_size);
}

void read_fat(uint32_t fat_start_sector, uint32_t fat_size_sectors, uint8_t *fat_table) {
    // Read the FAT table from the disk into memory
    disk_read(fat_start_sector, fat_size_sectors, fat_table, fat_size_sectors * SECTOR_SIZE);
}



void list_directory(fat_directory_entry_t *root_dir, uint16_t num_entries) {
    for (int i = 0; i < num_entries; i++) {
        if (root_dir[i].name[0] == 0x00) break;  // End of directory
        if (root_dir[i].name[0] == 0xE5) continue;  // Deleted entry

        // Print the filename (first 8 characters + 3 characters extension)
        char name[12];
        strncpy(name, root_dir[i].name, 11);
        name[11] = '\0';
        print(name);
        print("\n");
    }
}

void read_file(fat_directory_entry_t *file_entry, uint8_t *buffer) {
    uint32_t cluster = file_entry->first_cluster_low;
    uint32_t file_size = file_entry->file_size;
    uint32_t cluster_size = boot_sector.bytes_per_sector * boot_sector.sectors_per_cluster;

    while (file_size > 0) {
        // Read the current cluster
        uint32_t sector = cluster_to_sector(cluster);
        disk_read(sector, boot_sector.sectors_per_cluster, buffer, cluster_size);

        // Move to the next cluster
        cluster = get_next_cluster(cluster);
        file_size -= cluster_size;
        buffer += cluster_size;
    }
}
