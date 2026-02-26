#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "fat32_mount.h"
#include "cluster_manager.h"

typedef struct {
    uint32_t first_cluster;     // First Cluster number
    uint32_t size;              // file size
    uint32_t pos;               // pointer position
    uint32_t parent_cluster;    // Parent Directory cluster
    char name[11];              // 8.3 Name
} FAT32_FILE;                   // 104 bytes


bool create_fat32_volume( uint64_t start_lba, uint32_t sectors); // defined in fat32_mount.c
bool fat32_mount( uint64_t partition_lba_start);                 // defined in fat32_mount.c

bool fat32_change_current_directory( const char *path); 
bool fat32_mkdir( const char* dirpath);

bool fat32_open( const char *path, FAT32_FILE *file);

uint32_t fat32_read( FAT32_FILE *file, void *buffer, uint32_t size);        // read file
uint32_t fat32_write( FAT32_FILE *file, const void *buffer, uint32_t size); // write file







