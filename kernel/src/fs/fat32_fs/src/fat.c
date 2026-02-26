
#include "../../../lib/string.h"
#include "../../../lib/stdlib.h"
#include "../../../lib/stdio.h"
#include "../../../lib/ctype.h"

#include "../include/fat32_utility.h"

#include "../include/fat.h"



#define SECTOR_SIZE 512
#define MAX_BATCH_SECTORS 64  // Write 32KB at a time

bool initialize_fat_tables(uint64_t fat_start, uint32_t fat_sector_size) {
    
    uint8_t *fat_buffer = (uint8_t *) malloc( SECTOR_SIZE);
    if (!fat_buffer) {
        return false;
    }
    memset(fat_buffer, 0, SECTOR_SIZE);
    
    // Initialize first two FAT entries 64-bit FAT entries for FAT32
    // FAT[0] = 0x0FFFFFF8 : Media descriptor + reserved bits
    fat_buffer[0] = 0xF8;  // Media descriptor
    fat_buffer[1] = 0xFF;  // Reserved
    fat_buffer[2] = 0xFF;  // Reserved
    fat_buffer[3] = 0x0F;  // End of first cluster chain

    // FAT[1] = 0x0FFFFFFF : Reserved cluster (used to indicate bad cluster or end of chain)
    fat_buffer[4] = 0xFF;  // Reserved
    fat_buffer[5] = 0xFF;  // Reserved
    fat_buffer[6] = 0xFF;  // Reserved
    fat_buffer[7] = 0x0F;  // End of second cluster chain

    // FAT[2] = 0x0FFFFFFF : Root Directory Cluster : Mark cluster 2 as end of chain (since it's the root directory and currently empty)
    fat_buffer[8] = 0xFF;  // Reserved
    fat_buffer[9] = 0xFF;  // Reserved
    fat_buffer[10] = 0xFF; // Reserved
    fat_buffer[11] = 0x0F; // Reserved


    if(!fat32_write_sector(fat_start, fat_buffer)){   // Writing first three FAT entries
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
        if (!fat32_write_sectors( fat_start + sectors_written, sectors_to_write, zeros)) {
            free(zeros);
            return false;
        }
        sectors_written += sectors_to_write;
    }

    free(zeros);

    return true;
}









