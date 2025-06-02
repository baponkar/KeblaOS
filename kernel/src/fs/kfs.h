#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../sys/ahci/ahci.h"


#define MAX_FILES      64
#define BLOCK_SIZE     512
#define DIR_ENTRY_SIZE 128    // 512 / 4 entries per block
#define DIR_BLOCKS     16
#define DATA_START     17

typedef struct {
    char name[100];           // File name (null-terminated)
    uint32_t start_block;     // First data block
    uint32_t size_bytes;      // File size in bytes
    uint8_t used;             // 1 = used, 0 = free
} __attribute__((packed)) DirEntry;

typedef struct {
    uint32_t magic;           // e.g., 0x4B465331 = "KFS1"
    uint32_t total_blocks;    // total size of the FS
    uint32_t free_block;      // next free block for allocation
} __attribute__((packed)) Superblock;


bool kfs_init(HBA_PORT_T* port, uint32_t total_blocks);

bool kfs_create(HBA_PORT_T* port, const char* name, const void* data, uint32_t size);
int kfs_read(HBA_PORT_T* port, const char* name, void* out_buf);

void kfs_test(HBA_PORT_T* port);




