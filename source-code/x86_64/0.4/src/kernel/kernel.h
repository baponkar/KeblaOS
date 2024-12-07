#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// https://github.com/limine-bootloader/limine/blob/v8.x/PROTOCOL.md#kernel-address-feature
#include "../util/util.h"
#include "../../limine/limine.h"

#include "../driver/vga/vga.h"
#include "../driver/keyboard.h"

#include "../usr/shell.h"

#include "../gdt/gdt.h"
#include "../idt/idt.h"
#include "../idt/timer.h"

#include "../mmu/paging.h"

extern size_t SCREEN_WIDTH;
extern size_t SCREEN_HEIGHT;

extern size_t MIN_LINE_NO;
extern size_t MAX_LINE_NO;

extern size_t MIN_COLUMN_NO;
extern size_t MAX_COLUMN_NO;

extern uint32_t *fb_ptr;
extern struct limine_memmap_response* memmap_info;

extern uint64_t kernel_physical_base;
extern uint64_t kernel_virtual_base;
extern uint64_t kernel_offset;

// Getting system information by limine
void get_framebuffer_info();
void print_memory_map();
void print_firmware_type();
void print_kernel_address();
void print_cpu_info();
void print_disk_info();
void print_paging_mode();
void print_bootloader_info();
void print_stack_info();
void print_hhdm();
void hcf(void);
void get_framebuffer_info();
void print_memory_map();

