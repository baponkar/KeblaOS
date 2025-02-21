
#include "../mmu/vmm.h"
#include "../bootloader/boot.h"
#include "../lib/string.h"
#include "../lib/stdio.h"

#include "fs.h"




void disk_read_sector(uint32_t sector, void* buffer);
void disk_write_sector(uint32_t sector, const void* buffer);



// Read Boot Sector
int fat32_read_boot_sector(fat32_fs_t* fs, uint32_t lba) {
    fat32_boot_sector_t* boot_sector = (fat32_boot_sector_t*)vm_alloc(512);
    if (!boot_sector) return -1;

    disk_read_sector(lba, boot_sector);

    // Validate boot sector
    if (boot_sector->boot_signature != 0x29) {
        vm_free(boot_sector);
        return -1;
    }

    memcpy(&fs->boot_sector, boot_sector, sizeof(fat32_boot_sector_t));
    vm_free(boot_sector);

    // Calculate important offsets
    fs->fat_start_sector = lba + fs->boot_sector.reserved_sectors;
    fs->sectors_per_fat = fs->boot_sector.sectors_per_fat_32;
    fs->data_start_sector = fs->fat_start_sector + (fs->boot_sector.num_fats * fs->sectors_per_fat);
    fs->root_dir_sector = fs->data_start_sector + ((fs->boot_sector.root_cluster - 2) * fs->boot_sector.sectors_per_cluster);
    fs->total_clusters = fs->boot_sector.total_sectors_32 / fs->boot_sector.sectors_per_cluster;

    return 0;
}

// Read FAT Entry
uint32_t fat32_read_fat_entry(fat32_fs_t* fs, uint32_t cluster) {
    uint32_t fat_sector = fs->fat_start_sector + (cluster * 4 / fs->boot_sector.bytes_per_sector);
    uint32_t fat_offset = (cluster * 4) % fs->boot_sector.bytes_per_sector;

    uint8_t* fat_buffer = (uint8_t*)vm_alloc(512);
    if (!fat_buffer) return 0;

    disk_read_sector(fat_sector, fat_buffer);
    uint32_t fat_entry = *(uint32_t*)(fat_buffer + fat_offset) & 0x0FFFFFFF;

    vm_free(fat_buffer);
    return fat_entry;
}

// Read File Data
int fat32_read_file(fat32_fs_t* fs, uint32_t start_cluster, void* buffer, size_t size) {
    uint32_t current_cluster = start_cluster;
    uint8_t* data_buffer = (uint8_t*)buffer;
    size_t bytes_read = 0;

    while (current_cluster < 0x0FFFFFF8 && bytes_read < size) {
        uint32_t sector = fs->data_start_sector + (current_cluster - 2) * fs->boot_sector.sectors_per_cluster;
        for (uint32_t i = 0; i < fs->boot_sector.sectors_per_cluster && bytes_read < size; i++) {
            disk_read_sector(sector + i, data_buffer + bytes_read);
            bytes_read += fs->boot_sector.bytes_per_sector;
        }
        current_cluster = fat32_read_fat_entry(fs, current_cluster);
    }

    return bytes_read;
}

// Main Function (Example Usage)
int main() {
    fat32_fs_t fs;
    if (fat32_read_boot_sector(&fs, 0) != 0) {
        printf("Failed to read boot sector\n");
        return -1;
    }

    uint8_t* file_buffer = (uint8_t*)vm_alloc(1024);
    if (!file_buffer) {
        printf("Failed to allocate memory\n");
        return -1;
    }

    // Assume we know the starting cluster of a file
    uint32_t start_cluster = 2; // Example: Root directory
    fat32_read_file(&fs, start_cluster, file_buffer, 1024);

    // Process file data...

    vm_free(file_buffer);
    return 0;
}