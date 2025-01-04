#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../lib/stdio.h"

#include "../limine/limine.h"
#include "../util/util.h"   
#include "../driver/vga.h"
#include "../x86_64/gdt/gdt.h"
#include "../x86_64/idt/idt.h"
#include "../x86_64/pit/pit_timer.h"
#include "../driver/keyboard.h"
#include "../mmu/kheap.h"
#include "../mmu/paging.h"
#include "../mmu/pmm.h"



void kmain(void);






