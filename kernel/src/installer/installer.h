#pragma once

#include <stdint.h>
#include <string.h>
#include <stdbool.h>


#include "../driver/disk/ahci/ahci.h"



// Limine MBR structure
typedef struct {
    uint8_t bootstrap[440];        // Bootstrap code area
    uint32_t disk_signature;       // Optional disk signature
    uint16_t reserved;             // Reserved (usually 0)
    uint8_t partition_table[64];   // 4 partition entries
    uint16_t boot_signature;       // 0xAA55
} __attribute__((packed)) LimineMBR;

// Partition entry for MBR
typedef struct {
    uint8_t status;                // 0x80 = bootable, 0x00 = non-bootable
    uint8_t chs_start[3];          // CHS start address
    uint8_t type;                  // Partition type (0xEE for GPT protective, or 0x83 for Linux)
    uint8_t chs_end[3];           // CHS end address
    uint32_t lba_start;           // LBA start (little endian)
    uint32_t lba_size;            // Number of sectors (little endian)
} __attribute__((packed)) MBRPartition;


bool install_kebla_os(int boot_disk_no, int disk_no);















