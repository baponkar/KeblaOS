#pragma once

#include "../stdlib/stdint.h"
#include "heap.h"
#include "../util/util.h"


void enable_paging(uint32_t* page_directory);
void setup_paging();
uint32_t create_page_directory_entry(uint32_t* page_table);
uint32_t create_page_table_entry(uint32_t physical_address);
void set_page_directory_entry(uint32_t *page_directory, uint32_t index, uint32_t *page_table);
void set_page_table_entry(uint32_t *page_table, uint32_t index, uint32_t physical_address);

void page_fault_handler(registers_t * regs);
void test_valid_paging();
void test_invalid_paging();