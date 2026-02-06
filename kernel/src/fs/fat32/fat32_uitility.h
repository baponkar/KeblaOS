#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


#include "fat32.h"


// File attributes: 
#define ATTR_READ_ONLY  0x01 
#define ATTR_HIDDEN     0x02
#define ATTR_SYSTEM     0x04 
#define ATTR_VOLUME_ID  0x08
#define ATTR_DIRECTORY  0x10
#define ATTR_ARCHIVE    0x20
#define ATTR_LONG_NAME  0x0F

typedef struct __attribute__((packed)) {
    char DIR_Name[11];          // Offset 0
    uint8_t DIR_Attr;           // Offset 11
    uint8_t DIR_NTRes;          // Offset 12
    uint8_t DIR_CrtTimeTenth;   // Offset 13
    uint16_t DIR_CrtTime;       // Offset 14
    uint16_t DIR_CrtDate;       // Offset 16
    uint16_t DIR_LstAccDate;    // Offset 18
    uint16_t DIR_FstClusHI;     // Offset 20
    uint16_t DIR_WrtTime;       // Offset 22
    uint16_t DIR_WrtDate;       // Offset 24
    uint16_t DIR_FstClusLO;     // Offset 26
    uint32_t DIR_FileSize;      // Offset 28
} FAT32_DirectoryEntry;         // 32 bit or 4 bytes




uint32_t fat32_get_next_cluster(int disk_no, uint32_t current_cluster);
bool fat32_set_next_cluster(int disk_no, uint32_t current_cluster, uint32_t next_cluster);

bool fat32_allocate_cluster(int disk_no, uint32_t *allocated_cluster);
bool fat32_allocate_cluster_chain(int disk_no, uint32_t count, uint32_t *first_cluster);

bool fat32_clear_cluster(int disk_no, uint32_t cluster);
bool fat32_free_cluster_chain(int disk_no, uint32_t start_cluster);

bool fat32_read_cluster(int disk_no, uint32_t cluster_number, void *buffer);
bool fat32_read_cluster_chain(int disk_no, uint32_t start_cluster, void *buffer, uint32_t max_bytes);

bool fat32_write_cluster(int disk_no, uint32_t cluster_number, const void *buffer);
bool fat32_write_cluster_chain(int disk_no, const void *buffer, uint32_t size, uint32_t *first_cluster);

uint32_t fat32_count_cluster_chain(int disk_no, uint32_t start_cluster);

bool fat32_append_cluster(int disk_no, uint32_t start_cluster, uint32_t *new_cluster);
bool fat32_validate_cluster_chain(int disk_no, uint32_t start_cluster);

bool fat32_find_free_dir_entry(int disk_no, uint32_t dir_cluster, uint32_t *out_cluster, uint32_t *out_offset);
void fat32_format_83_name(const char *name, char out[11]);

bool fat32_create_dir_entry(int disk_no, uint32_t parent_cluster, const char *name, uint8_t attr, uint32_t first_cluster , uint32_t file_size);

bool fat32_init_directory( int disk_no,  uint32_t dir_cluster, uint32_t parent_cluster );
bool fat32_mkdir_internal(int disk_no, uint32_t parent_cluster, const char *name) ;

bool fat32_dir_exists(int disk_no, uint32_t dir_cluster, const char *name);
bool fat32_mkdir_root(int disk_no, const char *name);

bool fat32_create_test_file(int disk_no);










