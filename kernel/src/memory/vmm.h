/*
https://chatgpt.com/share/675fa60d-c044-8001-aef6-d23b3d62ab62
*/

#pragma once

#include <stdint.h>
#include <stdbool.h>

enum allocation_type {
    ALLOCATE_CODE = 0x1,   // Allocate for code
    ALLOCATE_DATA = 0x2,   // Allocate for data
    ALLOCATE_STACK = 0x4,  // Allocate for stack
};

void vm_alloc(uint64_t va, uint8_t type);
void vm_free(uint64_t *ptr);

uint64_t phys_to_vir(uint64_t pa);
uint64_t vir_to_phys(uint64_t va);

bool is_phys_addr(uint64_t addr);
bool is_virt_addr(uint64_t addr);

void test_vmm();
void test_vmm_1();
