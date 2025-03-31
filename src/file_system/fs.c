
#include "../memory/kheap.h"
#include "../bootloader/boot.h"
#include "../lib/string.h"
#include "../lib/stdio.h"
#include  "../ahci/ahci.h"

#include "fs.h"



// Helper: Read one 512-byte sector from disk using AHCI.
int disk_read_sector(uint32_t sector, void *buffer) {
    // Our AHCI driver expects LBA addressing.
    return ahci_read(sector, 1, buffer);
}

// Global variables to store FAT16 parameters.
static FAT16_BootSector bootSector;
static uint32_t rootDirSector;
static uint32_t rootDirSectors;

int fat16_init() {
    // Read boot sector (sector 0)
    if (disk_read_sector(0, (void *)&bootSector) != 0) {
        printf("FAT16: Failed to read boot sector\n");
        return -1;
    }

    // Verify boot sector signature.
    if (bootSector.bootSectorSignature != 0xAA55) {
        printf("FAT16: Invalid boot sector signature: %x\n", bootSector.bootSectorSignature);
        return -1;
    }

    // Calculate root directory size (in sectors)
    rootDirSectors = ((bootSector.rootEntryCount * 32) +
                      (bootSector.bytesPerSector - 1)) / bootSector.bytesPerSector;

    // The root directory starts after the reserved sectors and the FAT area.
    // First FAT starts immediately after reserved sectors.
    // Total sectors used by FAT area = numFATs * FATSize16.
    rootDirSector = bootSector.reservedSectors + (bootSector.numFATs * bootSector.FATSize16);

    printf("FAT16: Boot sector loaded successfully.\n");
    printf("FAT16: Bytes/Sector: %d, Sectors/Cluster: %d\n",
           bootSector.bytesPerSector, bootSector.sectorsPerCluster);
    printf("FAT16: Root directory starts at sector: %d (spanning %d sectors)\n",
           rootDirSector, rootDirSectors);

    return 0;
}



void fat16_list_root_dir() {
    uint32_t totalEntries = bootSector.rootEntryCount;
    uint32_t bufSize = bootSector.bytesPerSector * rootDirSectors;
    char *buffer = kheap_alloc(bufSize);
    if (!buffer) {
        printf("FAT16: Memory allocation failed for root directory\n");
        return;
    }
    memset(buffer, 0, bufSize);

    // Read all sectors of the root directory.
    for (uint32_t i = 0; i < rootDirSectors; i++) {
        if (disk_read_sector(rootDirSector + i, buffer + i * bootSector.bytesPerSector) != 0) {
            printf("FAT16: Failed to read root directory sector %d\n", rootDirSector + i);
            kheap_free(buffer, bufSize);
            return;
        }
    }

    FAT16_DirEntry *entries = (FAT16_DirEntry *)buffer;
    for (uint32_t i = 0; i < totalEntries; i++) {
        // 0x00 indicates no more entries.
        if (entries[i].name[0] == 0x00)
            break;
        // 0xE5 indicates a deleted entry.
        if ((uint8_t)entries[i].name[0] == 0xE5)
            continue;
        // Skip volume labels.
        if (entries[i].attr & 0x08)
            continue;

        char name[12];
        memcpy(name, entries[i].name, 11);
        name[11] = '\0';
        // Trim trailing spaces.
        for (int j = 10; j >= 0; j--) {
            if (name[j] == ' ')
                name[j] = '\0';
            else
                break;
        }
        printf("FAT16: File: %s, Size: %d bytes, First Cluster: %d\n",
               name, entries[i].fileSize, entries[i].fstClusLO);
    }
    kheap_free(buffer, bufSize);
}
