#pragma once

#include "../stdlib/stdint.h"
#include "../mmu/heap.h"
#include "../mmu/paging.h"
#include "../idt/idt.h"
#include "../stdlib/stdio.h"
#include "../util/util.h"

typedef struct process {
    uint32_t pid;                     // Process ID
    page_directory_t *page_directory; // Pointer to the process's page directory
    struct process *next;             // Pointer to the next process (for scheduling)
    // Add additional fields as needed (e.g., state, priority)
} process_t;


process_t* create_process();
void init_process_paging(page_directory_t *dir);
void switch_process(process_t *process);
void scheduler();
void timer_interrupt_handler(registers_t *regs);

