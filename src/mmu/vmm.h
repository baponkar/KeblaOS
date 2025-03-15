/*
https://chatgpt.com/share/675fa60d-c044-8001-aef6-d23b3d62ab62
*/

#pragma once

#include <stdint.h>


void vm_alloc(uint64_t va);
void vm_free(uint64_t *ptr);
void test_vmm();

