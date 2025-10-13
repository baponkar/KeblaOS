
/*
Getting Memory Information from Limine Bootloader

Development Date: 19/04/2025

References:
    https://github.com/limine-bootloader/limine/blob/v9.x/PROTOCOL.md#kernel-address-feature
    https://wiki.osdev.org/Memory_Map_(x86)
*/

#include "../../../ext_lib/limine-9.2.3/limine.h" 
#include "../lib/stdio.h"

#include "detect_memory.h"

extern bool debug_on;

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

// Get Stack memory info
__attribute__((used, section(".requests")))
static volatile struct limine_stack_size_request stack_size_request = {
    .id = LIMINE_STACK_SIZE_REQUEST,
    .revision = 3,
    .stack_size = 16384 // 16KB stack size
};

// Get paging mode type info
__attribute__((used, section(".requests")))
static volatile struct limine_paging_mode_request paging_mode_request = {
    .id = LIMINE_PAGING_MODE_REQUEST,
    .revision = 3
};

// Get memory map info
__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 3
};

// Get Kernel Load address
__attribute__((used, section(".requests")))
static volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 3
};

// Get Higher half direct map offset 
__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 3
};

extern bool debug_on;

uint64_t STACK_MEM_SIZE;

// LIMINE_PAGING_MODE_X86_64_4LVL = 0
// LIMINE_PAGING_MODE_X86_64_5LVL = 1
uint64_t paging_mode;



uint64_t HHDM_OFFSET;     // Transform Virtual to Physical Offset Present at Higher Half Memory

// Kernel Address
uint64_t KERNEL_VIR_BASE;
uint64_t KERNEL_PHYS_BASE;
uint64_t KERNEL_OFFSET;

// Physical Memory Head Address which is updating by kmalloc and pmm
volatile uint64_t phys_mem_head;    // This head is updated by paging, kmalloc, 

// Usable Physical Memory
uint64_t USABLE_START_PHYS_MEM;
uint64_t USABLE_END_PHYS_MEM;
uint64_t USABLE_LENGTH_PHYS_MEM;

// Total Physical Memory space found in the device
uint64_t TOTAL_PHYS_MEMORY;

// Memory Information found from Limine Bootloader
size_t mem_entry_count;
struct limine_memmap_entry **mem_entries;

static const char* get_mem_type(uint64_t type) {
    switch (type) {
        case LIMINE_MEMMAP_USABLE: return "USABLE"; // 0
        case LIMINE_MEMMAP_RESERVED: return "RESERVED"; // 1
        case LIMINE_MEMMAP_ACPI_RECLAIMABLE: return "ACPI_RECLAIMABLE"; //2
        case LIMINE_MEMMAP_ACPI_NVS: return "ACPI_NVS"; // 3
        case LIMINE_MEMMAP_BAD_MEMORY: return "BAD_MEMORY"; // 4
        case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE: return "BOOTLOADER_RECLAIMABLE"; // 5
        // case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES: return "EXECUTABLE_AND_MODULES"; // 6
        case LIMINE_MEMMAP_KERNEL_AND_MODULES: return "KERNEL_AND_MODULES"; // 6 (deprecated, use EXECUTABLE_AND_MODULES)
        case LIMINE_MEMMAP_FRAMEBUFFER: return "FRAMEBUFFER"; // 7

        default: return "UNKNOWN";
    }
}


// Getting Start Stack Memory Size
void get_stack_mem_info(){
    if(stack_size_request.response == NULL){
        if(debug_on) printf("[MEMORY] ERROR: Getting limine_stack_size_request failed!\n");
        return;
    }

    STACK_MEM_SIZE = stack_size_request.stack_size;
    if(debug_on) printf(" [MEMORY] ERROR:Start Stack size : %x\n", STACK_MEM_SIZE);
}

// What type Paging mode started by limine
void get_paging_mode(){
    if(paging_mode_request.response == NULL){
        if(debug_on) printf("[Error] Memory: Getting limine_paging_mode_request failed!\n");
        return;
    }

    paging_mode = paging_mode_request.response->mode;

    if(paging_mode == LIMINE_PAGING_MODE_X86_64_4LVL)
        if(debug_on) printf(" [Memory] LIMINE_PAGING_MODE_X86_64_4LVL\n");
    if(paging_mode == LIMINE_PAGING_MODE_X86_64_5LVL)
        if(debug_on) printf(" [Memory] LIMINE_PAGING_MODE_X86_64_5LVL\n");
}

// The Physical and Virtual address where limine put the kernel
void get_kernel_address(){
    if(kernel_address_request.response == NULL){
        if(debug_on) printf("[Error] Memory: Getting limine_kernel_address_request failed!\n");
        return;
    }

    KERNEL_PHYS_BASE = kernel_address_request.response->physical_base;
    KERNEL_VIR_BASE = kernel_address_request.response->virtual_base;

    KERNEL_OFFSET = KERNEL_VIR_BASE - KERNEL_PHYS_BASE;

    if(debug_on) printf(" [Memory] Kernel position address: Virtual = %x, Physical = %x\n", KERNEL_VIR_BASE, KERNEL_PHYS_BASE);
}

// Get The Higher Half Direct Map Offset
void get_hhdm_offset(){
    if(hhdm_request.response == NULL){
        if(debug_on) printf("[Error] Memory: Getting limine_hhdm_request failed!\n");
        return;
    }

    HHDM_OFFSET = hhdm_request.response->offset;
}

// Getting Memory map 
void get_phys_mem_map(){
    if(memmap_request.response == NULL){
        if(debug_on) printf("[Error] Memory: limine_memmap_request.response is failed!\n");
        return;
    }

    mem_entry_count = (size_t) memmap_request.response->entry_count;
    mem_entries = memmap_request.response->entries;

    for(size_t i=0; i<mem_entry_count; i++){
        uint64_t base = mem_entries[i]->base;
        uint64_t length = mem_entries[i]->length;
        uint64_t type = mem_entries[i]->type;

        // printf(" [Memory] Physical Base: %x, Length: %x, Type: %s\n", base, length, get_mem_type(type));
    }
}

// Set usable memory map to use further
void set_usable_mem(){
    if(mem_entries == NULL){
        if(debug_on) printf("[Error] Memory: mem_entries is empty!\n");
        return;
    }

    for(size_t i=0; i<mem_entry_count; i++){
        uint64_t base = mem_entries[i]->base;
        uint64_t length = mem_entries[i]->length;
        uint64_t type = mem_entries[i]->type;

        if(type == LIMINE_MEMMAP_USABLE ){ // LIMINE_MEMMAP_USABLE = 1
            if(length > USABLE_LENGTH_PHYS_MEM){
                USABLE_START_PHYS_MEM = base;
                USABLE_LENGTH_PHYS_MEM = length;
                USABLE_END_PHYS_MEM = USABLE_START_PHYS_MEM + USABLE_LENGTH_PHYS_MEM;
            }
        }
    }

    // USABLE_START_PHYS_MEM = 0x100000;

    USABLE_START_PHYS_MEM &= 0xFFFFFFFFFFFFF000;    // Making 4KB aligned
    phys_mem_head = USABLE_START_PHYS_MEM;          // Set the physical memory head to the start of usable memory

    //Final usable_mem_length
    USABLE_LENGTH_PHYS_MEM = USABLE_END_PHYS_MEM - USABLE_START_PHYS_MEM;

    // printf(" [Memory] Usable Phys. memory => Start: %x, End: %x, Length: %x\n", 
    //     USABLE_START_PHYS_MEM, USABLE_END_PHYS_MEM, USABLE_END_PHYS_MEM);
}

// Getting total physical memory space present at the device
void get_total_phys_memory(){
    if(mem_entries == NULL){
        if(debug_on) printf("[Error] Memory: mem_entries is empty!\n");
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

    if(debug_on) printf(" [Memory] Total Device Memory = %x\n", TOTAL_PHYS_MEMORY);
}

// Get memory info and set some value for further use
void get_set_memory(){

    get_stack_mem_info();
    get_paging_mode();
    get_kernel_address();
    get_hhdm_offset();

    if(memmap_request.response  == NULL){
        printf("[Error] Memory: Getting limine_memmap_request failed!\n");
        return;
    }

    if(HHDM_OFFSET != 0){
        uint64_t kernel_phys_start_addr = HIGHER_HALF_START_ADDR - HHDM_OFFSET;
        uint64_t kernel_phys_end_addr = HIGHER_HALF_END_ADDR - HHDM_OFFSET;

        if(debug_on){
            printf(" [Memory] HHDM Offset: %x\n", HHDM_OFFSET);
            printf(" [Memory] Higher Half Start(Virtual): %x and End(Virtual): %x\n", HIGHER_HALF_START_ADDR, HIGHER_HALF_END_ADDR);
            printf(" [Memory] Higher Half Start(Physical): %x and End(Physical): %x\n", kernel_phys_start_addr, kernel_phys_end_addr);
        }
    }

    // Get Physical Memory map and set usable memory map
    get_phys_mem_map();
    set_usable_mem();
    get_total_phys_memory();
}







