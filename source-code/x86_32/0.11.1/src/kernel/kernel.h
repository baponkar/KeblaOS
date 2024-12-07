#pragma once

#include "../gdt/gdt.h"

#include "../idt/idt.h"
#include "../idt/timer.h"

#include "../driver/vga.h"
#include "../driver/keyboard.h"

#include "../usr/shell.h"

#include "../mmu/heap.h"
#include "../mmu/paging.h"

#include "osinfo.h"


void check_system();