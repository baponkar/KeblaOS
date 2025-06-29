
#include "fatfs.h"

extern HBA_PORT_T* g_ahci_port;  // Assume this is your active port
extern uint64_t FAT32_PARTITION_LBA;

void fatfs_init(HBA_PORT_T* port) {

	if(!port) {
		printf("Error: AHCI port is NULL\n");
		return;
	}

	g_ahci_port = port;

	// Partition entry starts at offset 0x1BE
    uint16_t mbr[256];
    ahci_read(port, 0, 0, 1, mbr);
    uint32_t lba_start = *(uint32_t *)((uint8_t *)mbr + 0x1BE + 8);
	FAT32_PARTITION_LBA = lba_start; // Use lba_start instead of hardcoded 2048

    DSTATUS stat = disk_initialize(0);
    if (stat & STA_NOINIT) {
        printf("Disk not initialized!\n");
        return;
    }
	
    FATFS fs;

	// Initialize the FatFs library
	FRESULT res = f_mount(&fs, "", 1);
	if (res != FR_OK) {
		printf("Error mounting FatFs: %d\n", res);
		return;
	}

	printf("FatFs mounted successfully on partition starting at LBA: %x\n", FAT32_PARTITION_LBA);
}

void test_fatfs() {
    FATFS fs;
    FIL file;
    char buffer[128];
    UINT br, bw;

    // Mount the filesystem
    FRESULT res = f_mount(&fs, "", 1);
    if (res != FR_OK) {
        printf("Mount failed: %d\n", res);
        return;
    }

    // Create and write to the file
    if (f_open(&file, "test.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
        const char* msg = "Hello from KeblaOS!";
        res = f_write(&file, msg, strlen(msg), &bw);
        if (res == FR_OK) {
            printf("Successfully wrote %x bytes to test.txt\n", bw);
        } else {
            printf("Write failed: %d\n", res);
        }
        f_close(&file);
    } else {
        printf("Failed to create file\n");
    }

    // Reopen the file for reading
    if (f_open(&file, "test.txt", FA_READ) == FR_OK) {
        res = f_read(&file, buffer, sizeof(buffer) - 1, &br);
        if (res == FR_OK) {
            buffer[br] = 0;
            printf("Read from file: %s\n", buffer);
        } else {
            printf("Read failed: %d\n", res);
        }
        f_close(&file);
    } else {
        printf("Failed to open file for reading\n");
    }
}



