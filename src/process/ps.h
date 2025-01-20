#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../driver/vga.h"
#include "../util/util.h"
#include "../mmu/kheap.h"
#include "../mmu/paging.h"


struct process {
    uint64_t pid;                // Process ID
    uint64_t cr3;                // Physical address of the PML4 table for this process
    registers_t *regs;            // Saved CPU state (registers structure you already defined)
    uint64_t *stack;              // Kernel stack pointer for this process
    uint8_t state;               // READY, RUNNING, BLOCKED, etc.
    struct process *next;        // Next process in the linked list
};
typedef struct process process_t;

process_t *create_process(void (*entry_point)());
void scheduler_tick();

void init_multitasking();


