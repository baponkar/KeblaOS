
#pragma once

#include <stdint.h>


extern char *BOOTLOADER_NAME;
extern char *BOOTLOADER_VERSION;

extern uint64_t STACK_SIZE;


void get_limine_info();
void print_limine_info();

void get_bootloader_info();
void print_bootloader_info();


