
#pragma once

#include <stdint.h>





extern char *BOOTLOADER_NAME;
extern char *BOOTLOADER_VERSION;

extern uint64_t STACK_SIZE;

extern char *LIMINE_PAGING_MODE;


void get_kernel_modules_info();
void print_kernel_modules_info();



void get_stack_info();
void print_stack_info();

void get_limine_info();
void print_limine_info();

void get_paging_mode_info();
void print_paging_mode_info();

void get_bootloader_info();
void print_bootloader_info();


