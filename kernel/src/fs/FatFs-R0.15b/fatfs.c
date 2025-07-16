
#include "fatfs.h"


extern HBA_PORT_T* port;  
extern uint64_t FAT32_PARTITION_LBA;

FATFS *fatfs;

void fatfs_init() {

	if(!port) {
		printf("[FATFS] AHCI port is NULL\n");
		return;
	}

	// Partition entry starts at offset 0x1BE
    uint16_t mbr[256];
    ahci_read(port, 0, 0, 1, mbr);
    uint32_t lba_start = *(uint32_t *)((uint8_t *)mbr + 0x1BE + 8);
	FAT32_PARTITION_LBA = lba_start;    // Use lba_start instead of hardcoded 2048
    printf("[FATFS] FAT32_PARTITION_LBA : %d\n", lba_start);

    DSTATUS stat = disk_initialize(0);
    if (stat & STA_NOINIT) {
        printf("[FATFS] Disk not initialized!\n");
        return;
    }
	
    fatfs = (FATFS *)kheap_alloc(sizeof(FATFS), ALLOCATE_DATA);

    memset((void *)fatfs, 0, sizeof(FATFS));
    
	// Initialize the FatFs library
	FRESULT res = f_mount(fatfs, "", 1);
	if (res != FR_OK) {
		printf("[FATFS] Error mounting FatFs: %d\n", res);
		return;
	}

	printf("[FATFS] FatFs mounted successfully on partition starting at LBA: %x\n", FAT32_PARTITION_LBA);
}

void test_fatfs() {

    FIL *file = (FIL *)kheap_alloc(sizeof(FIL), ALLOCATE_DATA);
    char buffer[128];
    UINT br, bw;

    // Create and write to the file
    if (f_open(file, "test.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
        const char* msg = "Hello from KeblaOS!";
        FRESULT res = f_write(file, msg, strlen(msg), &bw);
        if (res == FR_OK) {
            printf("[FATFS] Successfully wrote %x bytes to test.txt\n", bw);
        } else {
            printf("[FATFS] Write failed: %d\n", res);
        }
        f_close(file);
    } else {
        printf("[FATFS] Failed to create file\n");
    }

    // Reopen the file for reading
    if (f_open(file, "test.txt", FA_READ) == FR_OK) {
        FRESULT res = f_read(file, buffer, sizeof(buffer) - 1, &br);
        if (res == FR_OK) {
            buffer[br] = 0;
            printf("[FATFS] Read from file: %s\n", buffer);
        } else {
            printf("[FATFS] Read failed: %d\n", res);
        }
        f_close(file);
    } else {
        printf("[FATFS] Failed to open file for reading\n");
    }
}

void list_dir(const char *path) {
    DIR dir;
    FILINFO fno;

    FRESULT res = f_opendir(&dir, path);
    if (res != FR_OK) {
        printf("[FATFS] Failed to open dir: %s, error: %d\n", path, res);
        return;
    }

    while (1) {
        res = f_readdir(&dir, &fno);
        if (res != FR_OK) {
            printf("[FATFS] Read error: %d\n", res);
            break;
        }

        if (fno.fname[0] == 0) {
            printf("[FATFS] End of dir.\n");
            break;
        }

        // Dump first few raw bytes
        printf("[DEBUG] fname[0..3]: %x %x %x %x\n",
               fno.fname[0], fno.fname[1], fno.fname[2], fno.fname[3]);

        printf((fno.fattrib & AM_DIR) ? "[FATFS] DIR : %s\n" : "[FATFS] FILE: %s (%d bytes)\n",
               fno.fname, (unsigned long long)fno.fsize);
    }

    f_closedir(&dir);
}



