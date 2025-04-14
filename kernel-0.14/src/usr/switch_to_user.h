#pragma once

#include <stdint.h>


void switch_to_user_mode(uint64_t stack_addr, uint64_t code_addr);
void init_user_mode();

int is_user_mode();

__attribute__((naked, noreturn))
void switch_to_user_mode_1(uint64_t stack_addr, uint64_t code_addr);

__attribute__((naked, noreturn))
void enter_user_mode(uint64_t rip, uint64_t rsp);
