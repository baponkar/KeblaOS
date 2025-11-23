
#include "disk.h"

#include "../../lib/stdio.h"
#include "../../lib/string.h"

#include  "../../memory/vmm.h"
#include "../../memory/kheap.h"

#include "inspect_disk.h"


#define SECTOR_SIZE 512

void print_mbr(int disk_no) {
    uint8_t buf[SECTOR_SIZE];
    
    if (!kebla_disk_read(disk_no, 0, 1, buf)) {
        printf("[ERROR] Failed to read MBR\n");
        return;
    }

    printf("=== Master Boot Record (LBA 0) ===\n");
    mbr_partition_entry_t_1 *part = (mbr_partition_entry_t_1*)(buf + 446);

    for (int i = 0; i < 4; i++) {
        printf("Partition %d:\n", i+1);
        printf("  Boot flag: %x\n", part[i].boot_indicator);
        printf("  Type: %x\n", part[i].part_type);
        printf("  LBA start: %u\n", part[i].lba_start);
        printf("  Total sectors: %u\n", part[i].total_sectors);
    }

    uint16_t sig = *(uint16_t*)(buf + 510);
    printf("MBR Signature: %x\n", sig);
}

void print_vbr(int disk_no, uint32_t part_lba) {
    uint8_t buf[SECTOR_SIZE];
    if (!kebla_disk_read(disk_no, part_lba, 1, buf)) {
        printf("[ERROR] Failed to read VBR\n");
        return;
    }

    fat32_bpb_t *bpb = (fat32_bpb_t*)buf;

    printf("\n=== FAT32 Boot Sector (VBR @ LBA %u) ===\n", part_lba);
    printf("BytesPerSector: %u\n", bpb->BytsPerSec);
    printf("SectorsPerCluster: %u\n", bpb->SecPerClus);
    printf("ReservedSectors: %u\n", bpb->RsvdSecCnt);
    printf("Number of FATs: %u\n", bpb->NumFATs);
    printf("FAT size (sectors): %u\n", bpb->FATSz32);
    printf("Root Cluster: %u\n", bpb->RootClus);
    printf("FSInfo Sector: %u\n", bpb->FSInfo);
    printf("Backup Boot Sector: %u\n", bpb->BkBootSec);
}




void print_fsinfo(int disk_no, uint32_t part_lba, uint16_t fsinfo_sector) {
    uint8_t buf[SECTOR_SIZE];
    if (!kebla_disk_read(disk_no, part_lba + fsinfo_sector, 1, buf)) {
        printf("[ERROR] Failed to read FSInfo\n");
        return;
    }

    fsinfo_t *fs = (fsinfo_t*)buf;

    printf("\n=== FAT32 FSInfo (LBA %u) ===\n", part_lba + fsinfo_sector);
    printf("LeadSig: %x\n", fs->LeadSig);
    printf("StrucSig: %x\n", fs->StrucSig);
    printf("Free clusters: %u\n", fs->Free_Count);
    printf("Next free cluster: %u\n", fs->Nxt_Free);
    printf("TrailSig: %x\n", fs->TrailSig);
}

void print_data_region(int disk_no, uint32_t part_lba, fat32_bpb_t *bpb) {
    uint32_t FirstDataSector = bpb->RsvdSecCnt + (bpb->NumFATs * bpb->FATSz32);
    uint32_t DataRegionLBA = part_lba + FirstDataSector;

    printf("\n=== Data Region Info ===\n");
    printf("FirstDataSector (relative): %u\n", FirstDataSector);
    printf("Data Region starts at LBA: %u\n", DataRegionLBA);
    printf("Cluster #2 starts at LBA: %u\n", DataRegionLBA);
}


void inspect_fat32(int disk_no) {
    // 1. Print MBR
    print_mbr(disk_no);

    // Read partition 1 info from MBR
    uint8_t buf[SECTOR_SIZE];
    kebla_disk_read(disk_no, 0, 1, buf);
    mbr_partition_entry_t_1 *part = (mbr_partition_entry_t_1*)(buf + 446);
    uint32_t part_lba = part[0].lba_start;   // Partition #1 start LBA

    // 2. Print VBR
    kebla_disk_read(disk_no, part_lba, 1, buf);
    fat32_bpb_t *bpb = (fat32_bpb_t*)buf;
    print_vbr(disk_no, part_lba);

    // 3. Print FSInfo
    print_fsinfo(disk_no, part_lba, bpb->FSInfo);

    // 4. Print Data region layout
    print_data_region(disk_no, part_lba, bpb);
}











