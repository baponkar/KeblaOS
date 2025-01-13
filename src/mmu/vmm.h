/*
https://chatgpt.com/share/675fa60d-c044-8001-aef6-d23b3d62ab62
*/

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../bootloader/boot.h"

#include "paging.h"


struct vm_object{
    uintptr_t base;
    size_t length;
    size_t flags;
    struct vm_object* next;
};
typedef struct vm_object vm_object_t;

uint64_t phys_to_vir(uint64_t phy_addr);
uint64_t vir_to_phys(uint64_t vir_addr);

void init_vmm();
uint64_t convert_x86_64_vm_flags(size_t flags);
void* vmm_alloc(size_t length, size_t flags, void* arg);
void map_memory(void* pml4, void* phys, void* virt, size_t flags);
void test_vmm();



