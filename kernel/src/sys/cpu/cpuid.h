#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


void get_cpu_vendor(char *vendor);
void get_cpu_brand(char *brand);

uint64_t get_cpu_base_frequency();
uint32_t get_lapic_id_by_cpuid();

bool has_apic();
bool has_sse();
bool has_sse2();
bool has_avx();

bool has_fpu();
void enable_fpu_and_sse();

int getLogicalProcessorCount();
void print_cpu_vendor();
void print_cpu_brand();
void print_cpu_base_frequency();




