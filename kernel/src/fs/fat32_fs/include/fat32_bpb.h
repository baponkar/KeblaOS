#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// BIOS Parameter Block
typedef struct __attribute__((packed)) {
    uint8_t BS_jmpBoot[3];      // Offset 0 : Jump instruction to boot code 0xeb 0x58 0x90
    char BS_OEMName[8];         // Offset 3 : OEM Name (null-terminated string) KeblaOS
    uint16_t BPB_BytsPerSec;    // Offset 11 : Bytes per Sector (should be 512 for FAT32)
    uint8_t BPB_SecPerClus;     // Offset 13 : Sectors per Cluster (1, 2, 4, 8, 16, 32, 64, or 128)
    uint16_t BPB_RsvdSecCnt;    // Offset 14 : Reserved Sector Count (number of sectors before the first FAT, typically 32 for FAT32)
    uint8_t BPB_NumFATs;        // Offset 16 : Number of FAT tables (usually 2)
    uint16_t BPB_RootEntCnt;    // Offset 17 : Root Entry Count (0 for FAT32)
    uint16_t BPB_TotSec16;      // Offset 19 : Total Sectors (if zero, use BPB_TotSec32)
    uint8_t BPB_Media;          // Offset 21 : Media Descriptor (0xF8 for fixed disk)
    uint16_t BPB_FATSz16;       // Offset 22 : FAT Size in Sectors (if zero, use BPB_FATSz32)
    uint16_t BPB_SecPerTrk;     // Offset 24 : Sectors per Track (for CHS addressing, typically 63)
    uint16_t BPB_NumHeads;      // Offset 26 : Number of Heads (for CHS addressing, typically 255)
    uint32_t BPB_HiddSec;       // Offset 28 : Hidden Sectors (number of sectors before the start of the partition)
    uint32_t BPB_TotSec32;      // Offset 32 : Total Sectors (if BPB_TotSec16 is zero)

    // FAT32 Extended BIOS Parameter Block
    uint32_t BPB_FATSz32;       // Offset 36 : FAT Size in Sectors (number of sectors occupied by each FAT) 
    uint16_t BPB_ExtFlags;      // Offset 40 : Extended Flags (bit 7 indicates FAT mirroring, bits 0-3 indicate active FAT) 0
    uint16_t BPB_FSVer;         // Offset 42 : File System Version (should be 0 for FAT32)
    uint32_t BPB_RootClus;      // Offset 44 : Root Cluster (cluster number of the root directory, typically 2)
    uint16_t BPB_FSInfo;        // Offset 48 : FSInfo Sector Number (sector number of the FSInfo structure, typically 1)
    uint16_t BPB_BkBootSec;     // Offset 50 : Backup Boot Sector Number (sector number of the backup boot sector, typically 6)
    uint8_t BPB_Reserved[12];   // Offset 52 : Reserved (should be zero)
    uint8_t BS_DrvNum;          // Offset 64 : Drive Number (0x80 for fixed disk)
    uint8_t BS_Reserved1;       // Offset 65 : Reserved (should be zero)
    uint8_t BS_BootSig;         // Offset 66 : Boot Signature (0x29 indicates that the following three fields are valid)
    uint32_t BS_VolID;          // Offset 67 : Volume ID (a unique identifier for the volume, often generated randomly)
    char BS_VolLab[11];         // Offset 71 : Volume Label (null-terminated string, padded with spaces) "KeblaOS FAT32"
    char BS_FilSysType[8];      // Offset 82 : File System Type (null-terminated string, padded with spaces) "FAT32   "
    uint8_t boot_code[420];     // Offset 90 : Boot code (can be used for bootloader, but can be zeroed out for non-bootable volumes)
    uint16_t Signature;         // Offset 510 : Boot Sector Signature (should be 0xAA55)
} BPB;                          // 512 bytes

extern BPB *bpb;

BPB *create_bpb_fat32(uint32_t tot_sectors, uint8_t sectors_per_cluster, uint32_t start_sector);


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


