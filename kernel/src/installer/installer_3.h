#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


bool format_disk_and_install(int iso_disk_no, int boot_disk_no);
bool verify_installation(int disk_no, uint32_t start_lba);
