#pragma once

#include <stdint.h>

#define OS_NAME "KeblaOS"
#define OS_VERSION "0.11"
#define BUILD_DATE "16/12/2024"

extern uint64_t V_KMEM_LOW_BASE;
extern uint64_t V_KMEM_UP_BASE;

void kmain();
void mem_info();

