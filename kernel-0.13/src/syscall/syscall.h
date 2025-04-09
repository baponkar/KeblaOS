#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void syscall_handler(uint64_t syscall_num, uint64_t arg1);
void init_syscall();
