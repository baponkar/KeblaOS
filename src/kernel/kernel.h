#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../limine/limine.h"

#include "../bootloader/boot.h"

#include "../lib/stdio.h"

#include "../util/util.h"   

#include "../x86_64/gdt/gdt.h"
#include "../x86_64/idt/idt.h"

#include "../driver/vga.h"
#include "../x86_64/pit/pit_timer.h"
#include "../driver/keyboard.h"
#include "../mmu/kmalloc.h"
#include "../mmu/pmm.h"
#include "../mmu/paging.h"
#include "../mmu/vmm.h"
#include "../mmu/kheap.h"

#include "../process/ps.h"

#include "../driver/window.h"

#include "../driver/mouse.h"

extern uint64_t V_KMEM_LOW_BASE;
extern uint64_t V_KMEM_UP_BASE;

void kmain();
void mem_info();

