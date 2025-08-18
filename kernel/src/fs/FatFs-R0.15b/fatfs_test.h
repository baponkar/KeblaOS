#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "./source/ff.h"        // FatFs library header



void fatfs_list_dir(const char *path);
void test_fatfs();

char *get_parent_dir(const char *path);


