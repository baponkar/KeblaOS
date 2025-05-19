
#include "../lib/stdio.h"
#include "../lib/string.h"

#include "fat16.h"


static FAT16_BootSector bs;
static HBA_PORT_T* fat16_port = 0;
static uint32_t bytes_per_sector = 512;
static uint16_t sectors_per_cluster = 1;
static uint32_t root_dir_sectors = 0;
static uint32_t fat_start_lba = 0;
static uint32_t root_start_lba = 0;
static uint32_t data_start_lba = 0;

static uint16_t sector_buf[256];

bool fat16_init(HBA_PORT_T* port) {
    fat16_port = port;

    if (!ahci_read(port, 0, 0, 1, sector_buf)) {
        printf(" [-] FAT16: Failed to read boot sector\n");
        return false;
    }

    memcpy(&bs, sector_buf, sizeof(FAT16_BootSector));

    bytes_per_sector = bs.bytes_per_sector;
    sectors_per_cluster = bs.sectors_per_cluster;
    fat_start_lba = bs.reserved_sectors;
    root_start_lba = fat_start_lba + bs.num_fats * bs.fat_size_16;
    root_dir_sectors = ((bs.root_entry_count * 32) + (bytes_per_sector - 1)) / bytes_per_sector;
    data_start_lba = root_start_lba + root_dir_sectors;

    printf(" [+] FAT16 Initialized\n");
    return true;
}

void fat16_list_root() {
    uint16_t root_entries = bs.root_entry_count;
    uint16_t sectors_to_read = root_dir_sectors;

    for (uint16_t i = 0; i < sectors_to_read; i++) {
        if (!ahci_read(fat16_port, root_start_lba + i, 0, 1, sector_buf)) {
            printf(" [-] FAT16: Failed to read root directory\n");
            return;
        }

        FAT16_DirEntry* entries = (FAT16_DirEntry*) sector_buf;
        for (int j = 0; j < bytes_per_sector / sizeof(FAT16_DirEntry); j++) {
            if (entries[j].name[0] == 0x00) return;
            if ((entries[j].attr & 0x0F) == 0x0F) continue; // Skip LFN
            if (entries[j].name[0] == 0xE5) continue; // Deleted entry

            char name[12];
            memcpy(name, entries[j].name, 11);
            name[11] = '\0';
            for (int k = 0; k < 11; k++) {
                if (name[k] == ' ') name[k] = '\0';
            }

            printf("File: %s, Size: %d bytes\n", name, entries[j].file_size);
        }
    }
}

bool fat16_read_file(const char* filename, uint8_t* out_buf, uint32_t* out_size) {
    uint16_t root_entries = bs.root_entry_count;
    uint16_t sectors_to_read = root_dir_sectors;

    for (uint16_t i = 0; i < sectors_to_read; i++) {
        if (!ahci_read(fat16_port, root_start_lba + i, 0, 1, sector_buf)) return false;

        FAT16_DirEntry* entries = (FAT16_DirEntry*) sector_buf;
        for (int j = 0; j < bytes_per_sector / sizeof(FAT16_DirEntry); j++) {
            char entry_name[12];
            memcpy(entry_name, entries[j].name, 11);
            entry_name[11] = '\0';

            // Format filename to 8.3
            char formatted[12];
            memset(formatted, ' ', 11);
            const char* dot = strchr(filename, '.');
            if (dot) {
                memcpy(formatted, filename, dot - filename);
                memcpy(formatted + 8, dot + 1, strlen(dot + 1));
            } else {
                memcpy(formatted, filename, strlen(filename));
            }

            if (memcmp(entry_name, formatted, 11) == 0) {
                uint16_t cluster = entries[j].first_cluster_low;
                uint32_t size = entries[j].file_size;
                uint32_t bytes_read = 0;
                uint32_t cluster_size = sectors_per_cluster * bytes_per_sector;

                while (cluster >= 0x0002 && cluster < 0xFFF8) {
                    uint32_t lba = data_start_lba + (cluster - 2) * sectors_per_cluster;
                    for (uint16_t s = 0; s < sectors_per_cluster; s++) {
                        if (!ahci_read(fat16_port, lba + s, 0, 1, sector_buf)) return false;
                        memcpy(out_buf + bytes_read, sector_buf, 512);
                        bytes_read += 512;
                        if (bytes_read >= size) break;
                    }

                    // Read next cluster from FAT
                    uint32_t fat_offset = cluster * 2;
                    uint32_t fat_sector = fat_start_lba + (fat_offset / bytes_per_sector);
                    uint32_t fat_index = (fat_offset % bytes_per_sector) / 2;

                    if (!ahci_read(fat16_port, fat_sector, 0, 1, sector_buf)) return false;
                    cluster = ((uint16_t*)sector_buf)[fat_index];
                }

                *out_size = size;
                return true;
            }
        }
    }

    return false;
}
