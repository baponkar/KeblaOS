#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "../sys/ahci/ahci.h" // contains ahci_read/ahci_write and HBA_PORT_T

typedef struct {
    uint8_t  jump_boot[3];
    uint8_t  oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t  num_fats;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t  media;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint32_t fat_size_32;
    uint16_t ext_flags;
    uint16_t fs_version;
    uint32_t root_cluster;
    uint16_t fs_info;
    uint16_t backup_boot_sector;
    uint8_t  reserved[12];
    uint8_t  drive_number;
    uint8_t  reserved1;
    uint8_t  boot_signature;
    uint32_t volume_id;
    uint8_t  volume_label[11];
    uint8_t  fs_type[8];
} __attribute__((packed)) FAT32_BPB;

typedef struct {
    uint8_t  name[11];
    uint8_t  attr;
    uint8_t  nt_reserved;
    uint8_t  creation_time_tenth;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t first_cluster_high;
    uint16_t write_time;
    uint16_t write_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} __attribute__((packed)) FAT32_DirEntry;

typedef struct {
    uint32_t bytes_per_sector;
    uint32_t sectors_per_cluster;
    uint32_t reserved_sector_count;
    uint32_t num_fats;
    uint32_t fat_size;
    uint32_t fat_start_sector;
    uint32_t cluster_heap_start_sector;
    uint32_t root_dir_first_cluster;
} fat32_info_t;

typedef struct {
    char name[11];
    uint8_t attr;
    uint8_t reserved;
    uint8_t crtTimeTenth;
    uint16_t crtTime;
    uint16_t crtDate;
    uint16_t lstAccDate;
    uint16_t fstClusHI;
    uint16_t wrtTime;
    uint16_t wrtDate;
    uint16_t fstClusLO;
    uint32_t fileSize;
} __attribute__((packed)) DIR_ENTRY;

extern fat32_info_t fat32_info;

bool fat32_init(HBA_PORT_T* port);
bool fat32_read_root_dir();

bool fat32_create_file(const char* filename);
bool fat32_read_file(const char* filename, uint8_t* buffer, uint32_t max_size);
bool fat32_write_file(const char* filename, const uint8_t* data, uint32_t size);
bool fat32_delete_file(const char* filename);

uint32_t fat32_first_data_sector();
uint32_t fat32_fat_size();
uint32_t fat32_fat_start_sector();
uint32_t fat32_data_start_sector();

uint32_t fat32_cluster_to_sector(uint32_t cluster);
uint32_t fat32_sector_to_cluster(uint32_t sector);

uint32_t fat32_next_cluster(uint32_t cluster);
uint32_t fat32_read_cluster(uint32_t cluster, uint8_t* buffer, uint32_t size);
uint32_t fat32_write_cluster(uint32_t cluster, const uint8_t* buffer, uint32_t size);
uint32_t fat32_get_file_size(const char* filename);
uint32_t fat32_get_file_cluster(const char* filename);
uint32_t fat32_get_file_sector(const char* filename);
uint32_t fat32_get_file_offset(const char* filename);
uint32_t fat32_get_file_attributes(const char* filename);
uint32_t fat32_get_file_first_cluster(const char* filename);
uint32_t fat32_get_file_first_sector(const char* filename);
uint32_t fat32_get_file_first_offset(const char* filename); 
uint32_t fat32_allocate_cluster();
void     fat32_set_cluster(uint32_t cluster, uint32_t value);
uint32_t fat32_get_file_size(const char* filename);

bool fat32_create_directory(const char* name);
bool fat32_find_free_entry(uint32_t cluster, DIR_ENTRY* entry);
bool fat32_write_directory_entry(uint32_t cluster, DIR_ENTRY* entry);
bool fat32_init_directory_cluster(uint32_t cluster, uint32_t parent_cluster);
bool fat32_delete_directory(uint32_t cluster);
bool fat32_delete_directory_entry(uint32_t cluster, const char* name);

uint32_t fat32_get_directory_cluster(const char* name);
void     fat32_run_tests(HBA_PORT_T* global_port);



