#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../lib/string.h"
#include "../lib/stdio.h"               // printf

#include "../memory/kheap.h"
#include "../memory/vmm.h"



#include "../FatFs-R0.15b/source/ff.h"        // FatFs library header
#include "../FatFs-R0.15b/source/diskio.h"    // FatFs


void test_fatfs();

int create_file(const char* fname);

