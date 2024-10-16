#pragma once


#include "../gdt/gdt.h"

#include "../idt/idt.h"
#include "../idt/timer.h"

#include "../driver/vga.h"
#include "../driver/keyboard.h"


#include "../usr/shell.h"

#include "../mmu/paging.h"
#include "../mmu/scheduler.h"
#include "../mmu/task.h"


#include "osinfo.h"

// void simple_program();
void process1();
void process2();
