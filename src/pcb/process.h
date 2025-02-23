
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../util/util.h"

#define NAME_MAX_LEN 64

typedef enum {
    READY,
    RUNNING,
    DEAD
} status_t; // sizeof(status1_t) = 4 bytes


typedef struct process {
    size_t pid;                   // Offset 0
    status_t status;              // Offset 8
    registers_t* registers;       // Offset 16
    char name[NAME_MAX_LEN];      // Offset 24
    struct process* next;         // Offset 24 + NAME_MAX_LEN
} process_t;


extern process_t *current_process;
extern process_t *processes_list;
extern void restore_cpu_state(registers_t *regs);
extern void switch_to_process(process_t *process);

process_t* create_process(char* name, void(*function)(void*), void* arg);
void delete_process(process_t *prev_process, process_t *current_process);
registers_t* schedule(registers_t *regs);
void init_processes();
