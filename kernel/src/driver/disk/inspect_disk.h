#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


#pragma pack(push, 1)

// MBR Partition entry (16 bytes)
typedef struct {
    uint8_t  boot_indicator;   // 0x80 = bootable
    uint8_t  start_chs[3];     
    uint8_t  part_type;        // 0x0B or 0x0C = FAT32
    uint8_t  end_chs[3];
    uint32_t lba_start;        // first LBA of partition
    uint32_t total_sectors;    // size in sectors
} mbr_partition_entry_t_1;


// BIOS Parameter Block (FAT32)
typedef struct {
    uint8_t  jmpBoot[3];
    uint8_t  OEMName[8];
    uint16_t BytsPerSec;
    uint8_t  SecPerClus;
    uint16_t RsvdSecCnt;
    uint8_t  NumFATs;
    uint16_t RootEntCnt;
    uint16_t TotSec16;
    uint8_t  Media;
    uint16_t FATSz16;
    uint16_t SecPerTrk;
    uint16_t NumHeads;
    uint32_t HiddSec;
    uint32_t TotSec32;

    // FAT32 extended
    uint32_t FATSz32;
    uint16_t ExtFlags;
    uint16_t FSVer;
    uint32_t RootClus;
    uint16_t FSInfo;
    uint16_t BkBootSec;
    uint8_t  Reserved[12];
    uint8_t  DrvNum;
    uint8_t  Reserved1;
    uint8_t  BootSig;
    uint32_t VolID;
    uint8_t  VolLab[11];
    uint8_t  FilSysType[8];
} __attribute__((packed)) fat32_bpb_t;


// FSInfo sector (FAT32)
typedef struct {
    uint32_t LeadSig;     // 0x41615252
    uint8_t  Reserved1[480];
    uint32_t StrucSig;    // 0x61417272
    uint32_t Free_Count;  // last known free cluster count
    uint32_t Nxt_Free;    // next free cluster
    uint8_t  Reserved2[12];
    uint32_t TrailSig;    // 0xAA550000
} __attribute__((packed)) fsinfo_t;

#pragma pack(pop)



void print_mbr(int disk_no);
void print_vbr(int disk_no, uint32_t part_lba);
void print_fsinfo(int disk_no, uint32_t part_lba, uint16_t fsinfo_sector);
void print_data_region(int disk_no, uint32_t part_lba, fat32_bpb_t *bpb);

void inspect_fat32(int disk_no);







