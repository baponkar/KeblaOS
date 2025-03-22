#pragma once

#include "../limine/limine.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Getting CPU information using CPUID instruction
extern uint32_t is_cpuid_present(void);
static inline void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx);
int getLogicalProcessorCount();
void get_cpu_vendor(char *vendor);
void get_cpu_brand(char *brand);
uint32_t get_cpu_base_frequency();
uint32_t get_lapic_id_cpuid(void);
bool has_fpu();
void enable_fpu_and_sse();


// Getting CPU information using Limine Bootloader
int get_cpu_count();
void get_cpu_info();
void target_cpu_task(struct limine_smp_info *smp_info);

void switch_to_core(uint32_t target_lapic_id);
void start_bootstrap_cpu_core();
void start_secondary_cpu_cores(int start_id, int end_id);

void init_ap_stacks(int start_id, int end_id);

// Debugging
void print_cpu_vendor();
void print_cpu_brand();
void print_cpu_info();

