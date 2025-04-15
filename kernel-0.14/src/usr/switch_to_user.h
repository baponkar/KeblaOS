#pragma once

#include <stdint.h>



int is_user_mode();

__attribute__((naked, noreturn))
void switch_to_user_mode(uint64_t stack_addr, uint64_t code_addr);

void init_user_mode();