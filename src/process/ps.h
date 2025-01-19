#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../driver/vga.h"
#
#include "../mmu/kheap.h"

// Maximum number of processes
#define MAX_PROCESSES 64

// Process states
typedef enum {
    PROCESS_READY = 0,
    PROCESS_RUNNING = 1,
    PROCESS_WAITING = 2,
    PROCESS_TERMINATED = 3
} process_state_t;

// Process Control Block (PCB)
typedef struct process {
    uint64_t pid;                 // Process ID
    uint64_t *page_table;         // Page table for the process
    void *stack;                  // Pointer to the process stack
    process_state_t state;        // Current state of the process
    struct process *next;         // Next process in the scheduler
} process_t;


process_t *create_process(void (*entry_point)());
void terminate_process(process_t *process);
void switch_to_process(process_t *process);
void scheduler_tick();

void test_process1();
void test_process2();
void init_scheduler();

