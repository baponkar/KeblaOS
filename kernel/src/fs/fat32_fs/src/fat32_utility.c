
#include "../include/diskio.h"

#include "../include/fat32_utility.h"




bool fat32_read_sector(uint64_t lba, void *buf) {
    return fat32_disk_read(lba, 1, buf);
}

bool fat32_write_sector(uint64_t lba, const void *buf) {
    return fat32_disk_write(lba, 1, buf);
}

bool fat32_read_sectors(uint64_t lba, uint32_t count, void *buf) {
    return fat32_disk_read(lba, count, buf);
}

bool fat32_write_sectors(uint64_t lba, uint32_t count, const void *buf) {
    return fat32_disk_write(lba, count, buf);
}



