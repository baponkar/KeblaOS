
#include "../../../memory/kheap.h"
#include "../../../memory/vmm.h"
#include "../../../memory/kmalloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/diskio.h"
#include "../include/fat32_types.h"
#include "../include/fat32_bpb.h"
#include "../include/fat32_fsinfo.h"
#include "../include/fat.h"
#include "../include/fat32_utility.h"
#include "../include/cluster_manager.h"

#include "../include/fat32_mount.h"


uint32_t fat32_base_lba = 0;                // Base LBA of the FAT32 partition, set during mount or creation
extern uint32_t fat32_cwd_cluster;          // Defined in cluster_manager.c
extern BPB *bpb;                            // Defined in fat32_bpb.c
extern uint32_t fat32_free_cluster_no;
extern uint8_t *fat_buffer;
extern uint32_t fat_size_bytes;



/*
 This function sets the volume label in the root directory. 
 It either updates an existing Volume ID entry or creates a new one if it doesn't exist.
 */
static bool fat32_set_volume_label( const char *label) {
    uint32_t root_cluster = get_root_dir_cluster();
    uint32_t cluster_size = get_cluster_size_bytes();
    
    uint8_t *buf = (uint8_t *) malloc(cluster_size);
    if (!buf) return false;

    // 1. Read the first cluster of the root directory
    if (!fat32_read_cluster( root_cluster, buf)) {
        free(buf);
        return false;
    }

    // 2. Find an empty slot or an existing Volume ID slot
    DirEntry *target_entry = NULL;
    for (uint32_t offset = 0; offset < cluster_size; offset += 32) {
        DirEntry *entry = (DirEntry *)(buf + offset);
        
        // If we find an existing label, overwrite it. 
        // Otherwise, take the first available slot (0x00 or 0xE5).
        if (entry->DIR_Attr == ATTR_VOLUME_ID || entry->DIR_Name[0] == 0x00 || entry->DIR_Name[0] == 0xE5) {
            target_entry = entry;
            break;
        }
    }

    if (!target_entry) {
        free(buf);
        return false; // Root cluster is full (unlikely for a fresh disk)
    }

    // 3. Setup the Label Entry
    memset(target_entry, 0, sizeof(DirEntry));
    
    // Format name to 11 chars, no dot, uppercase, space padded
    for (int i = 0; i < 11; i++) {
        if (label[i] && label[i] != '.') {
            target_entry->DIR_Name[i] = toupper(label[i]);
        } else {
            target_entry->DIR_Name[i] = ' ';
        }
    }

    target_entry->DIR_Attr = ATTR_VOLUME_ID;
    target_entry->DIR_FstClusHI = 0; // Always 0 for Volume Labels
    target_entry->DIR_FstClusLO = 0; // Always 0 for Volume Labels
    target_entry->DIR_FileSize = 0;

    // 4. Write back to disk
    bool ok = fat32_write_cluster( root_cluster, buf);
    
    free(buf);

    return ok;
}

// Main function to create FAT32 volume
// This function create and write BPB, FSInfo, and initialize FAT tables
bool create_fat32_volume( uint32_t start_lba, uint32_t sectors) {

    printf("Creating FAT32 Volume at LBA %ld with %d sectors\n", start_lba, sectors);

    // fat32_base_lba = start_lba; // Set the base LBA for future operations

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

void fat32_reset() {
    fat32_base_lba = 0;
    fat32_cwd_cluster = 0;

    if(bpb) {
        free(bpb);
        bpb = NULL;
    }
}

extern bool kebla_disk_read(int disk_no, uint64_t lba, uint32_t count, void* buf);
extern int disk_no;

bool fat32_load_fat_cache() {

    uint32_t fat_start_lba = get_first_fat_sector();
    uint32_t fat_sectors = get_fat_size_in_sectors();
    uint32_t bytes_per_sector = get_bytes_per_sector();

    fat_size_bytes = fat_sectors * bytes_per_sector;

    printf("Loading FAT cache. Start LBA %d, Total Sectors: %d, Size %d Bytes\n", fat_start_lba, fat_sectors, fat_size_bytes);

    fat_buffer = (uint8_t *) phys_to_vir(kmalloc_a(fat_size_bytes, 1));
    if (!fat_buffer) return false;
    memset(fat_buffer, 0, fat_size_bytes);

  //  if (!fat32_read_sectors(fat_start_lba,  fat_sectors, fat_buffer)) {
  if (!kebla_disk_read(disk_no, fat_start_lba,  fat_sectors, fat_buffer)) {
        // free(fat_buffer);
        fat_buffer = NULL;
        return false;
    }

    // free(fat_buffer);

    return true;
}

bool fat32_mount( int disk_no, uint64_t start_lba, char *vol_label) {

    set_disk_no(disk_no);

    fat32_base_lba = start_lba;

    uint8_t *sector = (uint8_t *)malloc(512);
    if(!sector) return false;
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

    fat32_cwd_cluster = get_root_dir_cluster();

    /* Validate FAT32 */
    if (get_bytes_per_sector() != 512) {
        printf("FAT32: invalid sector size %d\n", get_bytes_per_sector());
        return false;
    }

    if (get_fat_size_in_sectors() == 0) {
        printf("FAT32: not FAT32\n");
        return false;
    }

    if (get_total_no_fat() == 0) {
        printf("FAT32: invalid FAT count\n");
        return false;
    }

    if (get_sectors_per_cluster() == 0) {
        printf("FAT32: invalid cluster size\n");
        return false;
    }

    if (get_total_sectors() <= 0){
        printf("FAT32: invalid total sectors\n");
        return false;
    }

    if(!fat32_set_volume_label(vol_label)){
        printf("FAT32: faile to set Volume Label\n");
        return false;
    }

    fat32_cwd_cluster = get_root_dir_cluster();

    if(!fat32_load_fat_cache()){
        printf("Failed to load FAT cache\n");
        return false;
    }
    
    printf("FAT32 mounted\n");
    printf(" Volume starts at LBA: %u\n", fat32_base_lba);
    printf(" Bytes/sector: %u\n", get_bytes_per_sector());
    printf(" Sectors/cluster: %u\n", get_sectors_per_cluster());
    printf(" Reserved sectors: %u\n", get_reserved_sector_count());
    printf(" FAT size: %u\n", get_fat_size_in_sectors());
    printf(" Root cluster: %u\n", get_root_dir_cluster());
    printf(" Total clusters: %u\n\n", get_total_clusters());

    return true;
}











