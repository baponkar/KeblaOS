
#include "fatfs.h"


uint64_t FAT32_PARTITION_LBA = 0;  // Start of FAT32 partition in sectors

FATFS *fatfs;



void fatfs_init() {

    fatfs = (FATFS *)kheap_alloc(sizeof(FATFS), ALLOCATE_DATA);
    memset((void *)fatfs, 0, sizeof(FATFS));

    // Initialize the FatFs library
	FRESULT res = f_mount(fatfs, "", 1);
	if (res != FR_OK) {
		printf("[FATFS] Error mounting FatFs: %d\n", res);
		return;
	}

    // Read the MBR to find the total sectors 
    uint16_t mbr[256];
    disk_read(0, (BYTE*) mbr, 0, 1);    // pdrv=0, buff=mbr, sector=0, count=1
    uint32_t lba_start = *(uint32_t *)((uint8_t *)mbr + 0x1BE + 8);
	FAT32_PARTITION_LBA = lba_start;    // Use lba_start instead of hardcoded 2048
    
	printf("[FATFS] FatFs mounted successfully on partition starting at LBA: %d\n", FAT32_PARTITION_LBA);
}



