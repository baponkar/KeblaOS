#pragma once
#include <stdint.h>
#include <string.h>
#include <stdbool.h>






bool is_keblaos_installed( int boot_disk_no);
void uefi_install(int boot_disk_no, int iso_disk_no);
