#pragma once

#include "../stdlib/stdio.h"
#include "../stdlib/stdint.h"

#include "../gdt/gdt.h"

#include "../idt/idt.h"
#include "../idt/timer.h"

#include "../driver/vga.h"
#include "../driver/keyboard.h"

#include "../usr/shell.h"

#include "../mmu/paging.h"

#include "../idt/tss.h"

#include "../bootloader/multiboot.h"

#include "syscall.h"

#include "osinfo.h"


void halt_cpu(void);
void check_system();

void format_size(uint64_t hex_value);
