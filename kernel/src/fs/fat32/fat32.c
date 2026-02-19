
#include "../../driver/disk/disk.h"

#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../lib/stdlib.h"



#include "fat32.h"



#define SECTOR_SIZE 512

#define SECTORS_PER_CLUSTER 16              // 8 KB

uint64_t fat32_base_lba = 0;

BPB *bpb = NULL;

static bool fat32_read_sector(int disk_no, uint64_t lba, void *buf) {
    return kebla_disk_read(disk_no, lba, 1, buf);
}

static bool fat32_write_sector(int disk_no, uint64_t lba, const void *buf) {
    return kebla_disk_write(disk_no, lba, 1, (void*)buf);
}


// This function creates and returns a BPB structure for FAT32
static BPB *create_bpb_fat32(uint32_t tot_sectors, uint8_t sectors_per_cluster, uint32_t start_sector){
    BPB *bpb = (BPB *)malloc(sizeof(BPB));
    if (!bpb) {
        return NULL;
    }

    memset(bpb, 0, sizeof(BPB));

    // Standard FAT32 BPB fields
    bpb->BS_jmpBoot[0] = 0xEB;
    bpb->BS_jmpBoot[1] = 0x58;
    bpb->BS_jmpBoot[2] = 0x90;
    memcpy(bpb->BS_OEMName, "KeblaOS", 7);
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
    bpb->BS_VolID = 0x12345678;  // Example Volume ID
    memcpy(bpb->BS_VolLab, "KeblaOS FAT32", 13);
    memcpy(bpb->BS_FilSysType, "FAT32   ", 8);
    bpb->Signature = 0xAA55;

    return bpb;
}

static FSInfo *create_fsinfo_sector(int disk_no, uint64_t start_lba) {
    FSInfo *fsinfo = (FSInfo *)malloc(sizeof(FSInfo));
    if (!fsinfo) {
        return NULL;
    }

    memset(fsinfo, 0, sizeof(FSInfo));

    fsinfo->FSI_LoadSig = 0x41615252;    // Lead signature
    fsinfo->FSI_StrucSig = 0x61417272;   // Structure signature
    fsinfo->FSI_Free_Count = 0xFFFFFFFF; // Unknown free cluster count
    fsinfo->FSI_Nxt_Free = 0xFFFFFFFF;   // Unknown next free cluster
    fsinfo->FSI_TrailSig = 0xAA550000;   // Trail signature

    return fsinfo;
}



#define MAX_BATCH_SECTORS 64  // Write 32KB at a time

static bool initialize_fat_tables(int disk_no, uint64_t fat_start, uint32_t fat_sector_size) {
    
    uint8_t *fat_buffer = (uint8_t *) malloc( SECTOR_SIZE);
    if (!fat_buffer) {
        return false;
    }
    memset(fat_buffer, 0, SECTOR_SIZE);
    
    // Initialize first two FAT entries 64-bit FAT entries for FAT32
    // FAT[0] = 0x0FFFFFF8
    fat_buffer[0] = 0xF8;  // Media descriptor
    fat_buffer[1] = 0xFF;  // Reserved
    fat_buffer[2] = 0xFF;  // Reserved
    fat_buffer[3] = 0x0F;  // End of first cluster chain

    // FAT[1] = 0x0FFFFFFF
    fat_buffer[4] = 0xFF;  // Reserved
    fat_buffer[5] = 0xFF;  // Reserved
    fat_buffer[6] = 0xFF;  // Reserved
    fat_buffer[7] = 0x0F;  // End of second cluster chain

    // FAT[2] = 0x0FFFFFFF : Root Directory Cluster
    fat_buffer[8] = 0xFF;  // Reserved
    fat_buffer[9] = 0xFF;  // Reserved
    fat_buffer[10] = 0xFF; // Reserved
    fat_buffer[11] = 0x0F; // Reserved


    if(!kebla_disk_write(disk_no, fat_start, 1, fat_buffer)){   // Writing first three FAT entries
        return false;
    }

    free(fat_buffer);

    uint8_t *zeros = (uint8_t *)malloc(MAX_BATCH_SECTORS * SECTOR_SIZE);
    if(!zeros){
        return false;
    }
    memset(zeros, 0, MAX_BATCH_SECTORS * SECTOR_SIZE);

    // Write initialized sectors
    uint32_t sectors_written = 1;
    while (sectors_written < fat_sector_size) {
        uint32_t sectors_to_write = (fat_sector_size - sectors_written > MAX_BATCH_SECTORS) ? MAX_BATCH_SECTORS : (fat_sector_size - sectors_written);
        if (!kebla_disk_write(disk_no, fat_start + sectors_written, sectors_to_write, zeros)) {
            free(zeros);
            return false;
        }
        sectors_written += sectors_to_write;
    }

    free(zeros);

    return true;
}




static bool fat32_zero_cluster(int disk_no, uint32_t cluster)
{
    uint32_t first_data_sector = bpb->BPB_RsvdSecCnt + (bpb->BPB_NumFATs * bpb->BPB_FATSz32);

    uint32_t first_sector = first_data_sector + (cluster - 2) * bpb->BPB_SecPerClus;

    uint64_t lba = fat32_base_lba + first_sector;

    uint32_t bytes = bpb->BPB_SecPerClus * SECTOR_SIZE;

    uint8_t *zero = malloc(bytes);
    if (!zero) return false;

    memset(zero, 0, bytes);

    bool ok = kebla_disk_write(disk_no, lba, bpb->BPB_SecPerClus, zero);

    free(zero);

    return ok;
}




// Main function to create FAT32 volume
// This function create and write BPB, FSInfo, and initialize FAT tables
bool create_fat32_volume(int disk_no, uint64_t start_lba, uint32_t sectors) {
    printf("Creating FAT32 Volume on Disk %d at LBA %d with %d sectors\n", disk_no, start_lba, sectors);

    fat32_base_lba = start_lba;

    // 1. Determine sectors per cluster based on volume size
    uint8_t sectors_per_cluster =  SECTORS_PER_CLUSTER; // Default to 32 KB clusters
    if (sectors < 65536) {          // Less than 32MB
        sectors_per_cluster = 1;    // 512B clusters
    } else if (sectors < 262144) {  // Less than 128MB
        sectors_per_cluster = 2;    // 1KB clusters
    } else if (sectors < 524288) {  // Less than 256MB
        sectors_per_cluster = 4;    // 2KB clusters
    }

    // 2. Create BPB
    bpb = create_bpb_fat32(sectors, sectors_per_cluster, start_lba);
    if (!bpb) {
        printf("Failed to create BPB structure.\n");
        return false;
    }

    // 3. Write Boot Sector into disk
    if (!fat32_write_sector(disk_no, start_lba, bpb)) {
        printf("Failed to write Boot Sector.\n");
        return false;
    }

    // Write backup boot sector at start_lba + 6
    if (!fat32_write_sector(disk_no, start_lba + 6, bpb)) {
        printf("Failed to write Backup Boot Sector.\n");
        return false;
    }

    // 4. Create FSInfo Sector
    FSInfo *fsinfo = create_fsinfo_sector(disk_no, start_lba + 1);
    if (!fsinfo) {
        printf("Failed to create FSInfo structure.\n");
        return false;
    }
    // Write FSInfo Sector into disk
    if (!fat32_write_sector(disk_no, start_lba + 1, fsinfo)) {
        printf("Failed to write FSInfo Sector.\n");
        return false;
    }

    free(fsinfo);

    // 5. Initialize FAT tables
    uint32_t fat_sector_size = bpb->BPB_FATSz32;
    for (uint32_t fat_num = 0; fat_num < bpb->BPB_NumFATs; fat_num++) {
        uint64_t fat_start = start_lba + bpb->BPB_RsvdSecCnt + (fat_num * fat_sector_size);
        printf("  Initializing FAT %d...", fat_num + 1);

        if (!initialize_fat_tables(disk_no, fat_start, fat_sector_size)) {
            printf("Failed to initialize FAT %d\n", fat_num + 1);
            return false;
        }
        printf(" Done\n");
    }


    // Initialize root directory cluster
    if (!fat32_zero_cluster(disk_no, bpb->BPB_RootClus)) {
        printf("Failed to initialize root directory cluster\n");
        return false;
    }

    return true;
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









