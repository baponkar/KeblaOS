
/*
Currently I am using Limine Bootloader, so I will use the Limine Bootloader protocol 
to get the bootloader information. The bootloader information is stored in the 
bootloader_info_request struct. The bootloader_info_request struct is defined in 
the kernel.c file. The bootloader_info_request struct contains the bootloader name
 and version. The bootloader name and version are stored in the name and version 
 fields of the bootloader_info_request struct.

 In future I will implement a custom bootloader and will use that .
*/


#include "boot.h"



__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(0);


__attribute__((used, section(".requests")))
static volatile struct limine_firmware_type_request _firmware_type_request = {
    .id = LIMINE_FIRMWARE_TYPE_REQUEST,
    .revision = 0
};

// Multiprocessor info

__attribute__((used, section(".requests")))
static volatile struct limine_smp_request _smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0
};


__attribute__((used, section(".requests")))
static volatile struct limine_paging_mode_request _paging_mode_request = {
    .id = LIMINE_PAGING_MODE_REQUEST,
    .revision = 0
};


__attribute__((used, section(".requests")))
static volatile struct limine_bootloader_info_request _bootloader_info_request = {
    .id = LIMINE_BOOTLOADER_INFO_REQUEST,
    .revision = 0
};


__attribute__((used, section(".requests")))
static volatile struct limine_stack_size_request _stack_size_request = {
    .id = LIMINE_STACK_SIZE_REQUEST,
    .revision = 0,
    .stack_size = 8192
};


__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request _memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 3
};

__attribute__((used, section(".requests")))
static volatile struct limine_kernel_address_request _kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 3
};

__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request _hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 3
};


struct limine_memmap_request memmap_request;
struct limine_hhdm_request hhdm_request;
struct limine_kernel_address_request kernel_address_request;

char *FIRMWARE_TYPE;

uint64_t CPU_COUNT;

char *BOOTLOADER_NAME;
char *BOOTLOADER_VERSION;

uint64_t STACK_SIZE;

uint64_t MULTIPROCESSOR_REVISION;
uint64_t MULTIPROCESSOR_OFFSET;

char *LIMINE_PAGING_MODE;


void get_firmware_info(void){
    if(_firmware_type_request.response != NULL){
        uint64_t firmware_type = _firmware_type_request.response->firmware_type;
        if(firmware_type == LIMINE_FIRMWARE_TYPE_X86BIOS){
            FIRMWARE_TYPE = "X86BIOS";
        }else if(firmware_type == LIMINE_FIRMWARE_TYPE_UEFI32){
            FIRMWARE_TYPE = "UEFI32";
        }else if(firmware_type == LIMINE_FIRMWARE_TYPE_UEFI64){
            FIRMWARE_TYPE = "UEFI64";
        } 
    }else{
        FIRMWARE_TYPE = "No firmware type found!";
    }
}



void get_stack_info(void){
    if(_stack_size_request.response != NULL){
        STACK_SIZE = _stack_size_request.stack_size;
    }else{
        STACK_SIZE = 0;
    }
}



void get_bootloader_info(void){
    memmap_request = _memmap_request;
    hhdm_request = _hhdm_request;
    kernel_address_request = _kernel_address_request;

    if(_bootloader_info_request.response != NULL){
        uint64_t revision = _bootloader_info_request.response->revision;
        BOOTLOADER_NAME = _bootloader_info_request.response->name;
        BOOTLOADER_VERSION = _bootloader_info_request.response->version;
    }else{
        BOOTLOADER_NAME = "No Bootloader Info found!";
        BOOTLOADER_VERSION = NULL;
    }
}



void get_paging_mode_info(void){
    if(_paging_mode_request.response != NULL){
        uint64_t mode = _paging_mode_request.response->mode;
        if(mode == LIMINE_PAGING_MODE_X86_64_4LVL){
            LIMINE_PAGING_MODE = "Limine Paging mode x86_64 4 Level";
        } else if(mode == LIMINE_PAGING_MODE_X86_64_5LVL){
            LIMINE_PAGING_MODE = "Limine Paging mode x86_64 5 Level";
        }
    }else{
        LIMINE_PAGING_MODE = "No Paging mode found!";
    }
}



void get_smp_info(void){
    if(_smp_request.response != NULL){
        uint64_t revision = _smp_request.response->revision;
        uint64_t flags = _smp_request.response->flags;
        uint64_t bsp_lapic_id = _smp_request.response->bsp_lapic_id; // The Local APIC ID of the bootstrap processor.
        CPU_COUNT = _smp_request.response->cpu_count; //  How many CPUs are present. It includes the bootstrap processor.
        struct limine_smp_info **cpus = _smp_request.response->cpus; // Pointer to an array of cpu_count pointers to struct limine_smp_info structures.
        for(size_t i=0;i<(size_t) CPU_COUNT;i++){
            uint64_t processor_id = cpus[0]->processor_id;
            uint64_t lapic_id = cpus[0]->lapic_id;
            uint64_t reserved = cpus[0]->reserved;
            limine_goto_address goto_address = cpus[0]->goto_address;
            uint64_t extra_argument = cpus[0]->extra_argument;
        }
    }else{
        CPU_COUNT = 0;
    }
}



void print_bootloader_info(){
    print("BOOTLOADER_NAME : ");
    print(BOOTLOADER_NAME);
    print("\n");

    print("BOOTLOADER_VERSION : ");
    print(BOOTLOADER_VERSION);
    print("\n");

    print("FIRMWARE_TYPE : ");
    print(FIRMWARE_TYPE);
    print("\n");


    print("LIMINE_PAGING_MODE : ");
    print(LIMINE_PAGING_MODE);
    print("\n");

    print("CPU_COUNT : ");
    print_dec(CPU_COUNT);
    print("\n");

    print("STACK_SIZE : ");
    print_dec(STACK_SIZE);
    print("\n");
}









