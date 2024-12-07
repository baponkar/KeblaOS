#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../limine/limine.h"
#include "../util/util.h"   

#include "../driver/vga.h"
#include "../x86_64/gdt.h"


void kmain(void);
void hcf(void);
void get_framebuffer_info(void);
void get_firmware_info(void);
void get_hhdm_info(void);
void get_stack_info(void);
void get_bootloader_info(void);
void get_paging_mode_info(void);
void get_smp_info(void);
void get_vir_to_phy_offset(void);
void print_memory_map(void);
