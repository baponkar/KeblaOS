#pragma once

#include <stdint.h>

struct process_t {
    uint64_t pid;
    uint64_t *page_table;       // Pointer to the process's PML4
    struct cpu_state *cpu_state; // CPU register state
    struct process_t *next;      // Next process in the task list
    // Add other attributes as needed (priority, scheduling info, etc.)
};
