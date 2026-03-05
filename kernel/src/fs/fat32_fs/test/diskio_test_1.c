#include <stdio.h>
#include <string.h>
#include <ctype.h>


#include "../include/diskio.h"

#include "diskio_test_1.h"


bool diskio_test(uint32_t lba) {
    uint8_t buffer[512];
    memset(buffer, 0, sizeof(buffer)); // Clear buffer before use
    uint32_t sector = lba; // Test with the given LBA

    // ClearDisk for testing
    if (!disk_write(sector, 1, buffer)) {
        printf("disk_clear failed\n");
        return false;
    }

    // Test disk_read
    if (!disk_read(sector, 1, buffer)) {
        printf("disk_read failed\n");
        return false;
    }
    printf("disk_read succeeded for sector %u\n", sector);

    // Test disk_write
    memset(buffer, 0xAB, sizeof(buffer)); // Fill buffer with test data
    if (!disk_write(sector, 1, buffer)) {
        printf("disk_write failed\n");
        return false;
    }
    printf("disk_write succeeded for sector %u\n", sector);

    // Verify the write by reading back the data
    uint8_t verify_buffer[512];
    if (!disk_read(sector, 1, verify_buffer)) {
        printf("disk_read failed during verification\n");
        return false;
    }
    if (memcmp(buffer, verify_buffer, sizeof(buffer)) != 0) {
        printf("Data verification failed after disk_write\n");
        return false;
    }
    printf("Data verification succeeded after disk_write for sector %u\n", sector);

    memset(buffer, 0, sizeof(buffer)); // Clear buffer before next test
    if(!disk_write(sector, 1, buffer)) {
        printf("disk_write failed\n");
        return false;
    }

    return true;
}