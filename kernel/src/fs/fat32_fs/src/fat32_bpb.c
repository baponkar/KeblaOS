
#include "../../../lib/string.h"
#include "../../../lib/stdlib.h"
#include "../../../lib/stdio.h"
#include "../../../lib/ctype.h"

#include "../include/fat32_bpb.h"

#define FAT32_OEM_NAME "KeblaOS"
#define FAT32_VOLUME_ID 0x12345678
#define FAT32_VOLUME_LABEL "KeblaOS FAT32"
#define FAT32_FILESYSTEM_TYPE "FAT32   "

#define SECTOR_SIZE 512
BPB *bpb = NULL;

// This function creates and returns a BPB structure for FAT32
BPB *create_bpb_fat32(uint32_t tot_sectors, uint8_t sectors_per_cluster, uint32_t start_sector){
    bpb = (BPB *)malloc(sizeof(BPB));
    if (!bpb) {
        return NULL;
    }
    
    memset(bpb, 0, sizeof(BPB));

    // Standard FAT32 BPB fields
    bpb->BS_jmpBoot[0] = 0xEB;
    bpb->BS_jmpBoot[1] = 0x58;
    bpb->BS_jmpBoot[2] = 0x90;
    memcpy(bpb->BS_OEMName, FAT32_OEM_NAME, 7);
    bpb->BPB_BytsPerSec = SECTOR_SIZE;
    bpb->BPB_SecPerClus = sectors_per_cluster;
    bpb->BPB_RsvdSecCnt = 32;  // Reserved Sector Count
    bpb->BPB_NumFATs = 2;      // Number of FAT 
    bpb->BPB_RootEntCnt = 0;   // 0 for FAT32
    bpb->BPB_TotSec16 = 0;     // Use 32-bit field
    bpb->BPB_Media = 0xF8;     // Fixed disk
    bpb->BPB_FATSz16 = 0;      // Use 32-bit field
    bpb->BPB_SecPerTrk = 63;   // Typical value
    bpb->BPB_NumHeads = 255;   // Typical value
    bpb->BPB_HiddSec = start_sector;        // Hidden sectors before the start of the partition
    bpb->BPB_TotSec32 = tot_sectors;        // Total Sectors

    // Calculation of FAT Size : How many  sectors are needed for a FAT table
    // First, calculate clusters without considering FATs
    uint32_t tmp_data_sectors = tot_sectors - bpb->BPB_RsvdSecCnt;
    uint32_t total_clusters = tmp_data_sectors / sectors_per_cluster;

    // Now calculate FAT size needed for these clusters
    // Each FAT entry is 4 bytes, each sector is 512 bytes
    uint32_t fat_size = (total_clusters * 4 + (SECTOR_SIZE - 1)) / SECTOR_SIZE; // Total sectors required to store FAT

    // Now recalculate with actual FAT size in sectors
    uint32_t data_sectors = tot_sectors - bpb->BPB_RsvdSecCnt - (bpb->BPB_NumFATs * fat_size);  // total_sectors - 32 - 2 
    total_clusters = data_sectors / sectors_per_cluster;
    fat_size = (total_clusters * 4 + (SECTOR_SIZE - 1)) / SECTOR_SIZE;                          // Round up

    bpb->BPB_FATSz32 = fat_size;    // Total Sectors is required to hold FAT
    bpb->BPB_ExtFlags = 0;
    bpb->BPB_FSVer = 0;
    bpb->BPB_RootClus = 2;  // Root directory starts at cluster 2
    bpb->BPB_FSInfo = 1;    // FSInfo sector is located at sector 1 from the start of the partition
    bpb->BPB_BkBootSec = 6; // Backup boot sector is located at sector 6 from the start of the partition
    memset(bpb->BPB_Reserved, 0, sizeof(bpb->BPB_Reserved));
    bpb->BS_DrvNum = 0x80;
    bpb->BS_BootSig = 0x29;
    bpb->BS_VolID = FAT32_VOLUME_ID;  // Example Volume ID
    memcpy(bpb->BS_VolLab, FAT32_VOLUME_LABEL, 11);
    memcpy(bpb->BS_FilSysType, FAT32_FILESYSTEM_TYPE, 8);
    bpb->Signature = 0xAA55;
    
    return bpb;
}



// Helper functions
uint32_t get_total_clusters() {
    if (!bpb) return 0;

    uint32_t total_sectors = bpb->BPB_TotSec32;
    uint32_t first_data_sector =  bpb->BPB_RsvdSecCnt + (bpb->BPB_NumFATs * bpb->BPB_FATSz32);

    uint32_t data_sectors = total_sectors - first_data_sector;

    return data_sectors / bpb->BPB_SecPerClus;
}

uint32_t get_root_dir_first_cluster(){
    if(!bpb) return 0;
    return bpb->BPB_RootClus;
}

uint8_t get_sectors_per_cluster(){
    if(!bpb) return 0;
    return bpb->BPB_SecPerClus;
}

uint16_t get_bytes_per_sector(){
    if(!bpb) return 0;
    return bpb->BPB_BytsPerSec;
}

uint32_t get_fat_size_in_sectors(){
    if(!bpb) return 0;
    return bpb->BPB_FATSz32;
}

uint32_t get_total_sectors(){
    if(!bpb) return 0;
    return bpb->BPB_TotSec32;
}

uint32_t get_first_data_sector(){
    if(!bpb) return 0;
    uint32_t first_data_sector = bpb->BPB_RsvdSecCnt + (bpb->BPB_NumFATs * bpb->BPB_FATSz32);   // Reserved Sectors + Total Sectors taken by two FATs
    return first_data_sector;
}

uint32_t get_first_sector_of_cluster(uint32_t cluster_number){
    if(!bpb) return 0;
    uint32_t first_data_sector = bpb->BPB_RsvdSecCnt + (bpb->BPB_NumFATs * bpb->BPB_FATSz32);
    uint32_t sectors_per_cluster = bpb->BPB_SecPerClus;
    uint32_t first_sector_of_cluster = first_data_sector + ((cluster_number - 2) * sectors_per_cluster);    // cluster number starts from 2
    return first_sector_of_cluster;
}

uint32_t get_first_dir_sect_num(){
    if(!bpb) return 0;
    uint32_t first_data_sector =  get_first_data_sector();
    uint32_t first_dir_sect_num = get_first_sector_of_cluster(first_data_sector);
    return first_dir_sect_num;
}

bool is_end_of_cluster_chain(uint32_t cluster_value){
    if(!bpb) return false;
    return (cluster_value >= 0x0FFFFFF8);
}

bool is_valid_cluster(uint32_t cluster_value){
    if(!bpb) return false;
    return (cluster_value >= 0x00000002 && cluster_value <= 0x0FFFFFEF);
}


uint32_t get_cluster_size_bytes(){
    if(!bpb) return 0;
    return bpb->BPB_BytsPerSec * bpb->BPB_SecPerClus;
}

