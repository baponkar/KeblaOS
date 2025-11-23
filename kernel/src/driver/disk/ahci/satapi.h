#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


#include "ahci.h"




void AtpiPortRebase(HBA_PORT_T *port);
bool runAtapiCommand(HBA_PORT_T *port, uint8_t *cdb, size_t cdb_len, uintptr_t buf_phys, uint32_t buf_size, bool write);

bool satapi_read(HBA_PORT_T *port, uint32_t lba, uint32_t sector_count, void *buffer);
bool satapi_write(HBA_PORT_T *port, uint32_t lba, uint32_t sector_count,void *buffer);

uint64_t satapi_get_total_sectors(HBA_PORT_T *port);
uint16_t satapi_get_bytes_per_sector(HBA_PORT_T *port);

bool satapi_check_media(HBA_PORT_T *port);

bool satapi_read_capacity(HBA_PORT_T *port, uint32_t *last_lba, uint32_t *sector_size);

bool satapi_inquiry(HBA_PORT_T *port);

bool satapi_eject(HBA_PORT_T *port);
bool satapi_load(HBA_PORT_T *port);

void test_satapi(HBA_PORT_T *port);




