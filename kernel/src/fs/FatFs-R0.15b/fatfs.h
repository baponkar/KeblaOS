
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../../lib/string.h"
#include "../../lib/stdio.h"        // printf

#include "../../memory/kheap.h"
#include "../../memory/vmm.h"

#include "./source/ff.h"        // FatFs library header
#include "./source/diskio.h"    // FatFs
#include "./source/ffconf.h"

#include "../../driver/disk/ahci/ahci.h"

void fatfs_init();
void test_fatfs();
void fatfs_list_dir(const char *path);


