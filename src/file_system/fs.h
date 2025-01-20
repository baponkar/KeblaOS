#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


#include "../driver/vga.h"
#include "../mmu/vmm.h"
#include "../bootloader/boot.h"


// FAT32 Boot Sector Structure
typedef struct {
    uint8_t  jump_code[3];
    char     oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  num_fats;
    uint16_t root_entries;
    uint16_t total_sectors_16;
    uint8_t  media_type;
    uint16_t sectors_per_fat_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint32_t sectors_per_fat_32;
    uint16_t flags;
    uint16_t version;
    uint32_t root_cluster;
    uint16_t fs_info_sector;
    uint16_t backup_boot_sector;
    uint8_t  reserved[12];
    uint8_t  drive_number;
    uint8_t  reserved1;
    uint8_t  boot_signature;
    uint32_t volume_id;
    char     volume_label[11];
    char     fs_type[8];
} __attribute__((packed)) fat32_boot_sector_t;

// FAT32 Filesystem Context
typedef struct {
    fat32_boot_sector_t boot_sector;
    uint32_t fat_start_sector;
    uint32_t data_start_sector;
    uint32_t root_dir_sector;
    uint32_t sectors_per_fat;
    uint32_t total_clusters;
} fat32_fs_t;


