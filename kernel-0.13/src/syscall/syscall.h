#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

enum {
    SYSCALL_PRINT = 1,
    SYSCALL_READ  = 2,
    SYSCALL_EXIT  = 3,
};

void syscall_handler(uint64_t syscall_num, uint64_t arg1, uint64_t arg2);
void syscall(uint64_t num, uint64_t arg1, uint64_t arg2);
void init_syscall();
