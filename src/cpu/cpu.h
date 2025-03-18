#pragma once

#include "../limine/limine.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


extern uint32_t is_cpuid_present(void);

void get_cpu_info();
void print_cpu_info();
int get_cpu_count();

void get_cpu_vendor(char *vendor);
void print_cpu_vendor();
void get_cpu_brand(char *brand);
void print_cpu_brand();
int getLogicalProcessorCount();

uint32_t get_cpu_base_frequency();


void enable_fpu_and_sse();
bool has_fpu();


void target_cpu_task(struct limine_smp_info *smp_info);
void switch_to_core(uint32_t target_lapic_id);
void start_secondary_cores1();
