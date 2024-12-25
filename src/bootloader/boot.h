#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../limine/limine.h"
#include "../driver/vga.h"

void get_firmware_info(void);
void get_hhdm_info(void);
void get_stack_info(void);
void get_bootloader_info(void);
void get_paging_mode_info(void);
void get_smp_info(void);
void print_bootloader_info();