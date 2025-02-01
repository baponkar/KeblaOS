#pragma once

#include <stdint.h>
#include "../util/util.h"   // for registers_t structure


#define NAME_MAX_LEN 64
#define STACK_SIZE (16 * 1024) // 16 KB stack size

typedef enum {
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_WAITING,
    THREAD_TERMINATED
} status_t;


typedef struct thread {
    uint64_t tid;                   // Thread ID
    char name[NAME_MAX_LEN];        // Thread name
    uint64_t *stack;                // Kernel stack for the thread
    registers_t registers;          // Saved CPU state
    status_t state;                 // Thread state (READY, RUNNING, etc.)
    struct thread *next;            // Pointer to the next thread in the process
} thread_t;


typedef struct process {
    uint64_t pid;                   // Process ID
    thread_t *threads;              // List of threads in the process
    struct process *next;           // Pointer to the next process
} process_t;


thread_t *create_thread(process_t *process, void (*entry_point)(void), char *name, void *arg);
void terminate_thread(process_t *process, thread_t *thread);
void context_switch(registers_t *regs);


process_t *create_process(char *name);
void terminate_process(process_t *process);
void schedule(registers_t *regs);
void init_processes();
void print_process_list();

void init_user_shell();
