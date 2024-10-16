#pragma once

#include "../stdlib/stdint.h"
#include "../driver/vga.h"
#include "../stdlib/string.h"

#define SECTOR_SIZE 512

// Structure of a FAT Boot Sector
typedef struct {
    uint8_t  jump_boot[3];          // Jump instruction to boot code
    char     oem_name[8];           // OEM Name identifier
    uint16_t bytes_per_sector;      // Bytes per sector (usually 512)
    uint8_t  sectors_per_cluster;   // Sectors per cluster
    uint16_t reserved_sectors;      // Number of reserved sectors (usually 1)
    uint8_t  num_fats;              // Number of FAT copies (usually 2)
    uint16_t root_entry_count;      // Number of root directory entries (FAT12/16)
    uint16_t total_sectors_16;      // Total sectors (if < 65536)
    uint8_t  media_type;            // Media type descriptor
    uint16_t fat_size_16;           // Sectors per FAT for FAT12/16
    uint16_t sectors_per_track;     // Sectors per track (disk geometry)
    uint16_t num_heads;             // Number of heads (disk geometry)
    uint32_t hidden_sectors;        // Hidden sectors before this partition
    uint32_t total_sectors_32;      // Total sectors (if FAT32 or more than 65535 sectors)

    // FAT32-specific fields
    uint32_t fat_size_32;           // Sectors per FAT (FAT32)
    uint16_t ext_flags;             // Extended flags (FAT32)
    uint16_t fs_version;            // File system version (FAT32)
    uint32_t root_cluster;          // Root directory starting cluster (FAT32)
    uint16_t fs_info;               // File system info sector number (FAT32)
    uint16_t backup_boot_sector;    // Backup boot sector location (FAT32)
    uint8_t  reserved[12];          // Reserved
    uint8_t  drive_number;          // Drive number (BIOS)
    uint8_t  reserved1;             // Reserved
    uint8_t  boot_signature;        // Extended boot signature (0x29)
    uint32_t volume_id;             // Volume serial number
    char     volume_label[11];      // Volume label string
    char     fs_type[8];            // File system type (FAT12, FAT16, FAT32)
} __attribute__((packed)) fat_boot_sector_t;

typedef struct {
    char name[11];      // File name (8 characters) + extension (3 characters)
    uint8_t attr;       // File attributes (e.g., directory, hidden)
    uint8_t reserved;   // Reserved
    uint8_t create_time_tenths;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t access_date;
    uint16_t first_cluster_high;
    uint16_t modify_time;
    uint16_t modify_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} __attribute__((packed)) fat_directory_entry_t;



void read_boot_sector(fat_boot_sector_t *boot_sector);
void mount_fat(fat_boot_sector_t *boot_sector);
void read_fat(uint32_t fat_start_sector, uint32_t fat_size_sectors, uint8_t *fat_table);
void list_directory(fat_directory_entry_t *root_dir, uint16_t num_entries);
void read_file(fat_directory_entry_t *file_entry, uint8_t *buffer);