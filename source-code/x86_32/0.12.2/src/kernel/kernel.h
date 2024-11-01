#pragma once

#include "../stdlib/stdio.h"
#include "../stdlib/stdint.h"

#include "../gdt/gdt.h"

#include "../idt/idt.h"
#include "../idt/timer.h"

#include "../driver/vga.h"
#include "../driver/keyboard.h"

#include "../usr/shell.h"

#include "../mmu/heap.h"
#include "../mmu/paging.h"

#include "../bootloader/multiboot.h"

#include "syscall.h"

#include "osinfo.h"


void kmain (uint32_t magic, uint32_t addr);
void init_system();
void halt_cpu();
void check_system();
void print_linker_pointer_info();
void print_mutiboot_info(uint32_t magic, uint32_t addr);
void get_cpu_info();

void print_execution_mode();
void user_program();
void start_user_program();