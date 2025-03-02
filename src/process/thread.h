#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "types.h"
#include "process.h"
#include "../util/util.h"


#define THREAD_NAME_MAX_LEN 64

struct thread {                     // Allocated size 0x70
    size_t tid;                     // Thread ID
    status_t status;                // Thread status
    char name[THREAD_NAME_MAX_LEN]; // Thread name
    process_t* parent;              // Reference to parent process
    struct thread* next;            // Linked list for threads
    uint64_t cpu_time;              // Track CPU time per thread
    registers_t *registers;         // Thread registers
};



extern size_t next_free_tid;

thread_t* create_thread(process_t* parent, const char* name, void (*function)(void*), void* arg);
void add_thread(thread_t* thread);
void remove_thread(thread_t* thread);
void delete_thread(thread_t* thread);



