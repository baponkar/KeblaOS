
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


// Disk type constants
typedef enum {
    DISK_TYPE_UNKNOWN = 0,
    DISK_TYPE_IDE_PATA = 1,
    DISK_TYPE_AHCI_SATA = 2,
    DISK_TYPE_NVME = 3,
    DISK_TYPE_SCSI = 4,
    DISK_TYPE_SATAPI = 5,
    DISK_TYPE_FLOPPY = 6,
    DISK_TYPE_RAID = 7,
    DISK_TYPE_SATA_COMPAT = 8,
    DISK_TYPE_SATA_VENDOR = 9,
    DISK_TYPE_SATA_GENERIC = 10,
    DISK_TYPE_SAS = 11,
    DISK_TYPE_IPI = 12
} DiskType;


typedef struct {
    bool initialized;               // Indicates if the disk has been initialized
    DiskType type;                  // Type of disk (AHCI, IDE, NVMe, etc.)
    uint16_t bytes_per_sector;      // Typically 512 or 4096
    uint64_t total_sectors;         // Total number of sectors

    uint32_t root_directory_sector; // Fixed for SATAPI
    uint32_t root_directory_size;   // Fixed for SATAPI
    uint32_t pvd_sector;            // Fixed for SATAPI

    void* context;                  // Driver-specific context (e.g., AHCI port, AHCI abar info)
} Disk;


extern Disk *disks;
extern int disk_count;


int  kebla_get_disks();
void kebla_disk_check();

bool kebla_disk_init(int disk_no);
bool kebla_disk_status(int disk_no);

bool kebla_disk_read(int disk_no, uint64_t lba, uint32_t count, void* buf);
bool kebla_disk_write(int disk_no, uint64_t lba, uint32_t count, void* buf);

int clear_disk(int disk_no, size_t *progress);

// Helper functions
int find_disk_type(int disk_no);
int get_total_disks();
void print_disk_sector(int disk_no, uint64_t lba, uint64_t count);


void kebla_disk_test(int disk_no);

