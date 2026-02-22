#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>



// Protective MBR Structure
typedef struct __attribute__((packed)) {
    uint8_t boot_code[440];
    uint32_t disk_signature;
    uint16_t reserved;
    struct __attribute__((packed)) {
        uint8_t status;
        uint8_t chs_start[3];
        uint8_t type;
        uint8_t chs_end[3];
        uint32_t lba_start;
        uint32_t sector_count;
    } partition[4]; // 64 byte
    uint16_t signature;
} ProtectiveMBR;    // 512 bytes

void create_protective_mbr(ProtectiveMBR *mbr, uint64_t total_sectors);

