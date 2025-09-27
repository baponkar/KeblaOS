#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


#include "ahci.h"




void AtpiPortRebase(HBA_MEM_T *abar, int port_no);
static bool runAtapiCommand(HBA_PORT_T *port, uint8_t *cdb, size_t cdb_len, uintptr_t buf_phys, uint32_t buf_size, bool write);

bool init_satapi();



bool satapi_inquiry(HBA_PORT_T *port);









