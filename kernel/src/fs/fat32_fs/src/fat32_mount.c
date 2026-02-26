
#include "../../../lib/string.h"
#include "../../../lib/stdlib.h"
#include "../../../lib/stdio.h"
#include "../../../lib/ctype.h"

#include "../include/fat32_bpb.h"
#include "../include/fat32_fsinfo.h"
#include "../include/fat.h"
#include "../include/fat32_utility.h"

#include "../include/fat32_mount.h"


uint32_t fat32_base_lba = 0;                // Base LBA of the FAT32 partition, set during mount or creation
extern uint32_t fat32_cwd_cluster;          // Defined in cluster_manager.c
extern BPB *bpb;                            // Defined in fat32_bpb.c

// Main function to create FAT32 volume
// This function create and write BPB, FSInfo, and initialize FAT tables
bool create_fat32_volume( uint64_t start_lba, uint32_t sectors) {
    printf("Creating FAT32 Volume at LBA %ld with %d sectors\n", start_lba, sectors);

    fat32_base_lba = start_lba; // Set the base LBA for future operations

    // 1. Determine sectors per cluster based on volume size
    uint8_t sectors_per_cluster =  16; // Default to 32 KB clusters
    if (sectors < 65536) {          // Less than 32MB
        sectors_per_cluster = 1;    // 512B clusters
    } else if (sectors < 262144) {  // Less than 128MB
        sectors_per_cluster = 2;    // 1KB clusters
    } else if (sectors < 524288) {  // Less than 256MB
        sectors_per_cluster = 4;    // 2KB clusters
    }

    // 2. Create BPB structure
    bpb = create_bpb_fat32(sectors, sectors_per_cluster, start_lba);
    if (!bpb) {
        printf("Failed to create BPB structure.\n");
        return false;
    }

    // 3. Write Boot Sector into disk
    if (!fat32_write_sector(start_lba, bpb)) {
        printf("Failed to write Boot Sector.\n");
        return false;
    }

    // Write backup boot sector at start_lba + 6
    if (!fat32_write_sector(start_lba + 6, bpb)) {
        printf("Failed to write Backup Boot Sector.\n");
        return false;
    }
    
    // 4. Create FSInfo Sector
    FSInfo *fsinfo = create_fsinfo_sector(start_lba + 1);
    if (!fsinfo) {
        printf("Failed to create FSInfo structure.\n");
        return false;
    }

    // Write FSInfo Sector into disk
    if (!fat32_write_sector(start_lba + 1, fsinfo)) {
        printf("Failed to write FSInfo Sector.\n");
        return false;
    }

    free(fsinfo);

    // 5. Initialize FAT tables
    uint32_t fat_sector_size = bpb->BPB_FATSz32;
    for (uint32_t fat_num = 0; fat_num < bpb->BPB_NumFATs; fat_num++) {
        uint64_t fat_start = start_lba + bpb->BPB_RsvdSecCnt + (fat_num * fat_sector_size);
        printf("  Initializing FAT %d...", fat_num + 1);

        if (!initialize_fat_tables(fat_start, fat_sector_size)) {
            printf("Failed to initialize FAT %d\n", fat_num + 1);
            return false;
        }
        printf(" Done\n");
    }

    // Initialize root directory cluster
    uint8_t empty_cluster[512 * bpb->BPB_SecPerClus];
    memset(empty_cluster, 0, sizeof(empty_cluster));
    if(!fat32_write_sectors(start_lba + bpb->BPB_RootClus, bpb->BPB_SecPerClus, empty_cluster)){
        printf("Failed to initialize root directory cluster\n");
        return false;
    }

    return true;
}

bool fat32_mount( uint64_t start_lba) {
    fat32_base_lba = start_lba;

    uint8_t sector[512];
    memset(sector, 0, sizeof(sector));

    if(!fat32_read_sector( start_lba, sector)){
        printf("FAT32: boot sector read failed\n");
        return false;
    }

    if(!bpb){
        bpb = malloc(sizeof(BPB));
        if (!bpb) {
            printf("FAT32: BPB alloc failed\n");
            return false;
        }
    }

    memcpy(bpb, sector, sizeof(BPB));   // Copy the boot sector data into the BPB structure

    /* Validate FAT32 */
    if (bpb->BPB_BytsPerSec != 512) {
        printf("FAT32: invalid sector size %d\n", bpb->BPB_BytsPerSec);
        return false;
    }

    if (bpb->BPB_FATSz32 == 0) {
        printf("FAT32: not FAT32\n");
        return false;
    }

    if (bpb->BPB_NumFATs == 0) {
        printf("FAT32: invalid FAT count\n");
        return false;
    }

    if (bpb->BPB_SecPerClus == 0) {
        printf("FAT32: invalid cluster size\n");
        return false;
    }

    fat32_cwd_cluster = bpb->BPB_RootClus;
    
    printf("FAT32 mounted\n");
    printf(" Volume starts at LBA: %u\n", fat32_base_lba);
    printf(" Bytes/sector: %u\n", bpb->BPB_BytsPerSec);
    printf(" Sectors/cluster: %u\n", bpb->BPB_SecPerClus);
    printf(" Reserved sectors: %u\n", bpb->BPB_RsvdSecCnt);
    printf(" FAT size: %u\n", bpb->BPB_FATSz32);
    printf(" Root cluster: %u\n", bpb->BPB_RootClus);
    printf(" Total clusters: %u\n", get_total_clusters());

    return true;
}
