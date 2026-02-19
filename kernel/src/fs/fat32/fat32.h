
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


// BIOS Parameter Block
typedef struct __attribute__((packed)) {

    uint8_t BS_jmpBoot[3];      // Offset 0
    char BS_OEMName[8];         // Offset 3
    uint16_t BPB_BytsPerSec;    // Offset 11
    uint8_t BPB_SecPerClus;     // Offset 13
    uint16_t BPB_RsvdSecCnt;    // Offset 14
    uint8_t BPB_NumFATs;        // Offset 16
    uint16_t BPB_RootEntCnt;    // Offset 17
    uint16_t BPB_TotSec16;      // Offset 19
    uint8_t BPB_Media;          // Offset 21
    uint16_t BPB_FATSz16;       // Offset 22
    uint16_t BPB_SecPerTrk;     // Offset 24
    uint16_t BPB_NumHeads;      // Offset 26
    uint32_t BPB_HiddSec;       // Offset 28
    uint32_t BPB_TotSec32;      // Offset 32

    // FAT32 Extended BIOS Parameter Block
    uint32_t BPB_FATSz32;       // Offset 36
    uint16_t BPB_ExtFlags;      // Offset 40
    uint16_t BPB_FSVer;         // Offset 42
    uint32_t BPB_RootClus;      // Offset 44
    uint16_t BPB_FSInfo;        // Offset 48
    uint16_t BPB_BkBootSec;     // Offset 50
    uint8_t BPB_Reserved[12];   // Offset 52
    uint8_t BS_DrvNum;          // Offset 64
    uint8_t BS_Reserved1;       // Offset 65
    uint8_t BS_BootSig;         // Offset 66
    uint32_t BS_VolID;          // Offset 67
    char BS_VolLab[11];         // Offset 71
    char BS_FilSysType[8];      // Offset 82
    uint8_t boot_code[420];     // Offset 90
    uint16_t Signature;         // Offset 510

} BPB;                          // 512 bytes


typedef struct __attribute__((packed)) {
    uint32_t FSI_LoadSig;        // Offset 0
    uint8_t reserved1[480];      // Offset 4
    uint32_t FSI_StrucSig;       // Offset 484
    uint32_t FSI_Free_Count;     // Offset 488
    uint32_t FSI_Nxt_Free;       // Offset 492
    uint8_t reserved2[12];       // Offset 496
    uint32_t FSI_TrailSig;       // Offset 508
} FSInfo;                        // 512 bytes

extern uint64_t fat32_base_lba;
extern BPB *bpb;

bool create_fat32_volume(int disk_no, uint64_t start_lba, uint32_t sectors);


// Helper functions
uint32_t get_total_clusters();
uint32_t get_root_dir_first_cluster();
uint8_t get_sectors_per_cluster();
uint16_t get_bytes_per_sector();
uint32_t get_fat_size_in_sectors();
uint32_t get_total_sectors();
uint32_t get_first_data_sector();
uint32_t get_first_sector_of_cluster(uint32_t cluster_number);
uint32_t get_first_dir_sect_num();
bool is_end_of_cluster_chain(uint32_t cluster_value);
bool is_valid_cluster(uint32_t cluster_value);
uint32_t get_cluster_size_bytes();


