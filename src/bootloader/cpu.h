#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <cpuid.h>

static inline void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx);
void get_cpu_info();
void print_cpu_info();

void get_cpu_vendor(char *vendor);
void print_cpu_vendor();
void get_cpu_brand(char *brand);
void print_cpu_brand();
int getLogicalProcessorCount();

uint32_t get_cpu_base_frequency();


void enable_fpu();
bool has_fpu();