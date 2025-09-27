
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    DISK_TYPE_NONE = 0,
    DISK_TYPE_SATA,
    DISK_TYPE_IDE,
    DISK_TYPE_NVME,
    DISK_TYPE_SCSI,
    DISK_TYPE_SATAPI
} DiskType;


typedef struct {
    DiskType type;              // Type of disk (AHCI, IDE, NVMe, etc.)
    uint16_t bytes_per_sector;  // Typically 512 or 4096
    uint64_t total_sectors;     // Total number of sectors
    void* context;              // Driver-specific context (e.g., AHCI port, AHCI abar info)
} Disk;

void find_all_disks();

int get_total_disks();

bool kebla_disk_init(int disk_no, DiskType type);

bool kebla_disk_status(int disk_no);

bool kebla_disk_read(int disk_no, uint64_t lba, uint32_t count, void* buf);
bool kebla_disk_write(int disk_no, uint64_t lba, uint32_t count, void* buf);

int detect_partition_table(int disk_no);

void disk_test(int disk_no);








