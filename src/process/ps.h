#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../driver/vga.h"
#include "../driver/ports.h"

#include "../mmu/kheap.h"
#include "../util/util.h"
#include "../lib/string.h"

#define NAME_MAX_LEN 64

enum status{
    READY,
    RUNNING,
    DEAD
};
typedef enum status status_t;


struct process{
    size_t pid;
    status_t status;
    registers_t *regs;
    char name[NAME_MAX_LEN];
    struct process* next;
};
typedef struct process process_t;


registers_t *schedule(registers_t *regs);

// void init_scheduler();

process_t* create_process(const char* name, void(*function)(void*), void* arg);
void delete_process(process_t *process);


void test_task1();
void test_task2();

