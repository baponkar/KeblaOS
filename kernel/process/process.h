
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "types.h"          // for process_t and thread_t structures
#include "../util/util.h"   // for registers_t

#define NAME_MAX_LEN 64

typedef enum { // 4 bytes
    READY,    // 0
    RUNNING,  // 1
    DEAD,     // 2
    SLEEPING  // 3
} status_t;


typedef struct process { // 112 byte
    size_t pid;                 // Process ID
    status_t status;            // Process status
    char name[NAME_MAX_LEN];    // Process name
    struct process* next;       // Linked list for processes

    thread_t* threads;          // List of threads in the process
    thread_t* current_thread;   // Current running thread
    
    uint64_t cpu_time;          // Track CPU time per process
} process_t;



extern size_t next_free_pid;        //Available free process id
extern process_t *current_process;  // Current running process
extern process_t *processes_list;   // List of all processes

process_t* create_process(const char* name);
void add_process(process_t* proc);
void remove_process(process_t* proc);
void delete_process(process_t* proc);

registers_t* schedule(registers_t* registers);

process_t* get_process_by_pid(size_t pid);
process_t * get_current_process();
void print_process_list();