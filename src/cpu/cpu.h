#pragma once

#include "../limine/limine.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


void init_cpu();
void set_ap_stacks(int start_id, int end_id);

// Getting SMP information using Limine Bootloader
uint64_t get_revision();
uint32_t get_flags();
int get_cpu_count();
int get_bsp_lapic_id();
uint32_t get_lapic_id_by_limine(int core_id);
struct limine_smp_info ** get_cpus();
limine_goto_address get_goto_address(int core_id);
uint64_t get_extra_argument(int core_id);


// Getting CPU information using Limine Bootloader
int get_cpu_count();
void get_cpu_info();
void target_cpu_task(struct limine_smp_info *smp_info);

void switch_to_core(uint32_t target_lapic_id);
void start_bootstrap_cpu_core();
void start_secondary_cpu_cores(int start_id, int end_id);

void set_ap_stacks(int start_id, int end_id);

// Debugging
void print_cpu_vendor();
void print_cpu_brand();
void print_cpu_info();

