#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../../pci/pci.h"

#include "ahci.h"


#define MAX_AHCI_DISK 10

bool init_sata();
bool sata_read(HBA_PORT_T* port, size_t _lba, size_t _count, uintptr_t _buf_phys_addr);
bool sata_write(HBA_PORT_T* port, size_t _lba, size_t _count, uintptr_t _phys_buf_addr);

uint64_t sata_get_total_sectors(HBA_PORT_T* port);
uint32_t sata_get_bytes_per_sector(HBA_PORT_T* port);

void sata_disk_identify(HBA_PORT_T* port);

void test_sata(int ahci_disk_no);






