#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "./source/ff.h"        // FatFs library header
#include "./source/diskio.h"    // FatFs Disk Input/Output

#include "../../lib/string.h"
#include "../../lib/stdio.h"        // printf

#include "../../memory/kheap.h"
#include "../../memory/vmm.h"

void fatfs_list_dir(const char *path);
void test_fatfs();

