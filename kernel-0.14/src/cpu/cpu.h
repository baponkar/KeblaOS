#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


void set_ap_stacks(int start_id, int end_id);

void target_cpu_task(struct limine_smp_info *smp_info);
void switch_to_core(uint32_t target_lapic_id);

void start_bootstrap_cpu_core();
void start_secondary_cpu_cores(int start_id, int end_id);
void init_all_cpu_cores();




