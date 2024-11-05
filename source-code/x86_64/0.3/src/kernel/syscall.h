#pragma once

#include "../driver/vga/vga.h"
#include <stdint.h>
#include "../util/util.h"


void syscall_handler(registers_t *regs);
// Function declarations
extern int sys_write(const char *str);
extern int sys_read(char *buffer);

void syscall_check();