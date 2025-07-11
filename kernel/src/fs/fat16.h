#ifndef FAT16_H
#define FAT16_H

#include <stdint.h>
#include <stdbool.h>

#include "../driver/disk/ahci/ahci.h"

typedef struct {
    uint8_t jump_boot[3];
    uint8_t oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t media;
    uint16_t fat_size_16;
    uint32_t total_sectors_32;
    uint32_t fat_start_lba;
    uint32_t root_start_lba;
    uint32_t data_start_lba;
} __attribute__((packed)) FAT16_BootSector;

typedef struct {
    uint8_t name[11];
    uint8_t attr;
    uint8_t reserved;
    uint8_t create_time_tenth;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t access_date;
    uint16_t first_cluster_high;
    uint16_t write_time;
    uint16_t write_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} __attribute__((packed)) FAT16_DirEntry;

bool fat16_init(HBA_PORT_T* port);
void fat16_list_root();
bool fat16_read_file(const char* filename, uint8_t* out_buf, uint32_t* out_size);

#endif
