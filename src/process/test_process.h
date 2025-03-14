#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "types.h"
#include "process.h"
#include "thread.h"

void init_processes();


void thread0_func(void *arg);
void thread1_func(void* arg);
void thread2_func(void* arg);

void print_all_threads_name(process_t *p);