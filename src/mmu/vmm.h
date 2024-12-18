/*
https://chatgpt.com/share/675fa60d-c044-8001-aef6-d23b3d62ab62
*/

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "paging.h"

#include "../lib/stdio.h"

void map_page(uint64_t virt_addr, uint64_t phys_addr, uint64_t flags, pml4_t *pml4);
void unmap_page(uint64_t virt_addr, pml4_t *pml4);
uint64_t vmm_alloc(uint64_t size, uint64_t flags);
void vmm_free(uint64_t virt_addr, uint64_t size);
void page_fault_handler1(registers_t *regs);


