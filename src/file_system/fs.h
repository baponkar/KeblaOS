#pragma once

#include <stdint.h>
#include <stdbool.h>


typedef struct {
    uint8_t  jump[3];
    uint8_t  oemName[8];
    uint16_t bytesPerSector;
    uint8_t  sectorsPerCluster;
    uint16_t reservedSectors;
    uint8_t  numFATs;
    uint16_t rootEntryCount;
    uint16_t totalSectors16;
    uint8_t  media;
    uint16_t FATSize16;
    uint16_t sectorsPerTrack;
    uint16_t numHeads;
    uint32_t hiddenSectors;
    uint32_t totalSectors32;
    // Extended Boot Record
    uint8_t  driveNumber;
    uint8_t  reserved1;
    uint8_t  bootSignature;
    uint32_t volumeID;
    uint8_t  volumeLabel[11];
    uint8_t  fileSystemType[8];
    uint8_t  bootCode[448];
    uint16_t bootSectorSignature; // Should be 0xAA55
} FAT16_BootSector;

typedef struct {
    uint8_t  name[11];     // 8.3 filename
    uint8_t  attr;         // File attributes
    uint8_t  ntRes;        // Reserved
    uint8_t  crtTimeTenth; // Creation time (tenths of a second)
    uint16_t crtTime;      // Creation time
    uint16_t crtDate;      // Creation date
    uint16_t lstAccDate;   // Last access date
    uint16_t fstClusHI;    // High word of first cluster (unused in FAT16)
    uint16_t wrtTime;      // Last write time
    uint16_t wrtDate;      // Last write date
    uint16_t fstClusLO;    // First cluster (low word)
    uint32_t fileSize;     // File size in bytes
} FAT16_DirEntry;


/// Initialize the FAT16 filesystem.
int fat16_init();

/// List files in the FAT16 root directory.
void fat16_list_root_dir();



