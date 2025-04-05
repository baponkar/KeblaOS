#pragma once

#include <stdint.h>

void init_syscall(void);
void syscall_dispatcher(void);
static inline uint64_t syscall(uint64_t num, uint64_t arg1, uint64_t arg2,
    uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);

    void uses_syscall();