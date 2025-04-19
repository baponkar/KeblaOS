
/*
Getting Memory Information from Limine Bootloader

Development Date: 19/04/2025

References:
    https://github.com/limine-bootloader/limine/blob/v8.x/PROTOCOL.md#kernel-address-feature
    https://wiki.osdev.org/Memory_Map_(x86)
*/

#include "../../../limine-8.6.0/limine.h" 
#include "../lib/stdio.h"

#include "detect_memory.h"


__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(0);

// Get Stack memory info
__attribute__((used, section(".requests")))
static volatile struct limine_stack_size_request stack_size_request = {
    .id = LIMINE_STACK_SIZE_REQUEST,
    .revision = 3,
    .stack_size = 16384
};

// Get paging mode type info
__attribute__((used, section(".requests")))
static volatile struct limine_paging_mode_request paging_mode_request = {
    .id = LIMINE_PAGING_MODE_REQUEST,
    .revision = 0
};

// Get memory map info
__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

// Get Kernel Load address
__attribute__((used, section(".requests")))
static volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

// Get Higher half direct map offset 
__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

uint64_t STACK_MEM_SIZE;

// LIMINE_PAGING_MODE_X86_64_4LVL = 0
// LIMINE_PAGING_MODE_X86_64_5LVL = 1
uint64_t paging_mode;

// Virtual Address
uint64_t LOWER_HALF_START_ADDR = 0x1000;
uint64_t LOWER_HALF_END_ADDR = 0x00007FFFFFFFFFFF;

uint64_t HIGHER_HALF_START_ADDR = 0xFFFF800000000000;
uint64_t HIGHER_HALF_END_ADDR = 0xFFFFFFFFFFFFFFFF;
uint64_t HHDM_OFFSET;     // Transform Virtual to Physical Offset Present at Higher Half Memory


// Kernel Address
uint64_t KERNEL_VIR_BASE;
uint64_t KERNEL_PHYS_BASE;
uint64_t KERNEL_OFFSET;

// Usable Physical Memory
uint64_t USABLE_START_PHYS_MEM;
uint64_t USABLE_END_PHYS_MEM;
uint64_t USABLE_LENGTH_PHYS_MEM;

// Total Physical Memory space found in the device
uint64_t TOTAL_PHYS_MEMORY;

// Memory Information found from Limine Bootloader
size_t mem_entry_count;
struct limine_memmap_entry **mem_entries;


// Getting Start Stack Memory Size
void get_stack_mem_info(){
    if(stack_size_request.response == NULL){
        printf("[Error] Memory: Getting limine_stack_size_request failed!\n");
        return;
    }

    STACK_MEM_SIZE = stack_size_request.stack_size;
    printf("[Info] Memory: Start Stack size : %x\n", STACK_MEM_SIZE);
}

// What type Paging mode started by limine
void get_paging_mode(){
    if(paging_mode_request.response == NULL){
        printf("[Error] Memory: Getting limine_paging_mode_request failed!\n");
        return;
    }

    paging_mode = paging_mode_request.response->mode;

    if(paging_mode == LIMINE_PAGING_MODE_X86_64_4LVL)
        printf("[Info] Memory: LIMINE_PAGING_MODE_X86_64_4LVL\n");
    if(paging_mode == LIMINE_PAGING_MODE_X86_64_5LVL)
        printf("[Info] Memory: LIMINE_PAGING_MODE_X86_64_5LVL\n");
}

// The Physical and Virtual address where limine put the kernel
void get_kernel_address(){
    if(kernel_address_request.response == NULL){
        printf("[Error] Memory: Getting limine_kernel_address_request failed!\n");
        return;
    }

    KERNEL_PHYS_BASE = kernel_address_request.response->physical_base;
    KERNEL_VIR_BASE = kernel_address_request.response->virtual_base;

    KERNEL_OFFSET = KERNEL_VIR_BASE - KERNEL_PHYS_BASE;

    printf("[Info] Memory: Kernel position address: Virtual = %x, Physical = %x\n", KERNEL_VIR_BASE, KERNEL_PHYS_BASE);
}

// Get The Higher Half Direct Map Offset
void get_hhdm_offset(){
    if(hhdm_request.response == NULL){
        printf("[Error] Memory: Getting limine_hhdm_request failed!\n");
        return;
    }

    HHDM_OFFSET = hhdm_request.response->offset;
}

// Getting Memory map 
void get_phys_mem_map(){
    if(memmap_request.response == NULL){
        printf("[Error] Memory: limine_memmap_request.response is failed!\n");
        return;
    }

    mem_entry_count = (size_t) memmap_request.response->entry_count;
    mem_entries = memmap_request.response->entries;

    for(size_t i=0; i<mem_entry_count; i++){
        uint64_t base = mem_entries[i]->base;
        uint64_t length = mem_entries[i]->length;
        uint64_t type = mem_entries[i]->type;

        printf(" [-] Memory: Physical Base: %x, Length: %x, Type: %d\n", base, length, type);
    }
}

// Set usable memory map to use further
void set_usable_mem(){
    if(mem_entries == NULL){
        printf("[Error] Memory: mem_entries is empty!\n");
        return;
    }

    for(size_t i=0; i<mem_entry_count; i++){
        uint64_t base = mem_entries[i]->base;
        uint64_t length = mem_entries[i]->length;
        uint64_t type = mem_entries[i]->type;

        if(type == LIMINE_MEMMAP_USABLE ){
            if(length > USABLE_LENGTH_PHYS_MEM){
                USABLE_START_PHYS_MEM = base;
                USABLE_LENGTH_PHYS_MEM = length;
                USABLE_END_PHYS_MEM = USABLE_START_PHYS_MEM + USABLE_LENGTH_PHYS_MEM;
            }
        }
    }

    printf("[Info] Memory: Usable Phys. memory => Start: %x, End: %x, Length: %x\n", 
        USABLE_START_PHYS_MEM, USABLE_END_PHYS_MEM, USABLE_END_PHYS_MEM);
}

// Getting total physical memory space present at the device
void get_total_phys_memory(){
    if(mem_entries == NULL){
        printf("[Error] Memory: mem_entries is empty!\n");
        return;
    }

    for(size_t i=0; i<mem_entry_count; i++){
        uint64_t base = mem_entries[i]->base;
        uint64_t length = mem_entries[i]->length;
        uint64_t type = mem_entries[i]->type;

        if(i == 0){
            TOTAL_PHYS_MEMORY = base;
        }
        TOTAL_PHYS_MEMORY += length;
    }

    printf("[Info] Memory: Total Device Memory = %x\n", TOTAL_PHYS_MEMORY);
}

// Get memory info and set some value for further use
void get_set_memory(){
    get_stack_mem_info();
    get_paging_mode();
    get_kernel_address();
    get_hhdm_offset();

    if(memmap_request.response  == NULL){
        printf("[Info] Memory: Getting limine_memmap_request failed!\n");
        return;
    }

    if(HHDM_OFFSET != 0){
        uint64_t kernel_phys_start_addr = HIGHER_HALF_START_ADDR - HHDM_OFFSET;
        uint64_t kernel_phys_end_addr = HIGHER_HALF_END_ADDR - HHDM_OFFSET;

        printf("[Info] Memory: HHDM Offset: %x\n", HHDM_OFFSET);
        printf("[Info] Memory: Higher Half Start(Virtual): %x and End(Virtual): %x\n", 
            HIGHER_HALF_START_ADDR, HIGHER_HALF_END_ADDR);
        printf("[Info] Memory: Higher Half Start(Physical): %x and End(Physical): %x\n", 
            kernel_phys_start_addr, kernel_phys_end_addr);
    }

    // Get Physical Memory map and set usable memory map
    get_phys_mem_map();
    set_usable_mem();
    get_total_phys_memory();
}







