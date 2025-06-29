
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../lib/string.h"
#include "../lib/stdio.h"        // printf

#include "../memory/kheap.h"
#include "../memory/vmm.h"

#include "./source/ff.h"        // FatFs library header
#include "./source/diskio.h"    // FatFs

#include "../sys/ahci/ahci.h"

void fatfs_init(HBA_PORT_T* port);
void test_fatfs();



