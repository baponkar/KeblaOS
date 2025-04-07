#pragma once

#include "../../../limine-8.6.0/limine.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


void set_ap_stacks(int start_id, int end_id);

// Getting SMP information using Limine Bootloader
uint64_t get_revision();                            // Get Limine SMP revision
uint32_t get_flags();                               // Get Limine SMP flags
int get_cpu_count();                                // Get Limine CPU count
int get_bsp_lapic_id();                             // Get Limine BSP LAPIC ID
uint32_t get_lapic_id_by_limine(int core_id);       // Get Limine LAPIC ID by core ID
struct limine_smp_info ** get_cpus();               // Get Limine SMP info
limine_goto_address get_goto_address(int core_id);  // Get Limine goto address
uint64_t get_extra_argument(int core_id);           // Get Limine extra argument
void get_cpu_info();                                // Get Limine CPU info

void target_cpu_task(struct limine_smp_info *smp_info);
void switch_to_core(uint32_t target_lapic_id);

void start_bootstrap_cpu_core();
void start_secondary_cpu_cores(int start_id, int end_id);
void init_all_cpu_cores();

void set_ap_stacks(int start_id, int end_id);

// Debugging
void print_cpu_vendor();
void print_cpu_brand();
void print_cpu_info();

