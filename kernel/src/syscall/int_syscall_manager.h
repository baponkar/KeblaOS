#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

enum int_syscall_number{
    INT_SYSCALL_READ =  172,    // 0xAC - Read System Call
    INT_SYSCALL_PRINT = 173,    // 0xAD - Print System Call
    INT_SYSCALL_EXIT =  174,    // 0XAE - Exit System Call
};


void int_syscall_init();

void syscall_test(int syscall_no);



