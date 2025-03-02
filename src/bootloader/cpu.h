#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void get_cpu_info();
void print_cpu_info();

void get_cpu_vendor(char *vendor);
void print_cpu_vendor();
void get_cpu_brand(char *brand);
void print_cpu_brand();
int getLogicalProcessorCount();
