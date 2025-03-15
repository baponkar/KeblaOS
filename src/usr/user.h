
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>



void switch_to_user_mode(uint64_t stack_addr, uint64_t code_addr);