#pragma once

#include <stdint.h>


void switch_to_user_mode(uint64_t stack_addr, uint64_t code_addr);
void init_user_mode();


