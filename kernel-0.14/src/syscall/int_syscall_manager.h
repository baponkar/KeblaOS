#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

enum int_syscall_number{
    INT_SYSCALL_PRINT = 170,
    INT_SYSCALL_READ  = 171,
    INT_SYSCALL_EXIT  = 172,
};


void int_syscall_init();

char *syscall_test(int syscall_no);
