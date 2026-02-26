#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>



// FSInfo Sector Structure
typedef struct __attribute__((packed)) {
    uint32_t FSI_LoadSig;        // Offset 0 : FSInfo Structure Signature (should be 0x41615252)
    uint8_t reserved1[480];      // Offset 4 : Reserved (should be zero)
    uint32_t FSI_StrucSig;       // Offset 484 : FSInfo Structure Signature (should be 0x61417272)
    uint32_t FSI_Free_Count;     // Offset 488 : Last known free cluster count (0xFFFFFFFF if unknown)
    uint32_t FSI_Nxt_Free;       // Offset 492 : Cluster number of the next free cluster (0xFFFFFFFF if unknown)
    uint8_t reserved2[12];       // Offset 496 : Reserved (should be zero)
    uint32_t FSI_TrailSig;       // Offset 508 : Trail Signature (should be 0xAA550000)
} FSInfo;                        // 512 bytes


FSInfo *create_fsinfo_sector();

