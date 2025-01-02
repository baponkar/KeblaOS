
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
static volatile struct limine_bootloader_info_request _limine_info_request = {
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




char *FIRMWARE_TYPE;

uint64_t CPU_COUNT;

char *BOOTLOADER_NAME;
char *BOOTLOADER_VERSION;

uint64_t STACK_SIZE;

uint64_t MULTIPROCESSOR_REVISION;
uint64_t MULTIPROCESSOR_OFFSET;

char *LIMINE_PAGING_MODE;

uint64_t KERNEL_ADDRESS_REVISION;
uint64_t PHYSICAL_BASE;
uint64_t VIRTUAL_BASE;
uint64_t PHYSICAL_TO_VIRTUAL_OFFSET;

uint64_t HIGHER_HALF_DIRECT_MAP_REVISION;
uint64_t HIGHER_HALF_DIRECT_MAP_OFFSET;

struct limine_memmap_request memmap_request;
uint64_t entry_count;
struct limine_memmap_entry **entries;
uint64_t TOTAL_MEMORY;
uint64_t USABLE_MEMORY;
uint64_t RESERVED_MEMORY;
uint64_t BAD_MEMORY;
uint64_t BOOTLOADER_RECLAIMABLE_MEMORY;
uint64_t ACPI_RECLAIMABLE_MEMORY;
uint64_t ACPI_NVS_MEMORY;
uint64_t FRAMEBUFFER_MEMORY;
uint64_t KERNEL_MODULES_MEMORY;


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



void get_limine_info(void){
    if(_limine_info_request.response != NULL){
        uint64_t revision = _limine_info_request.response->revision;
        BOOTLOADER_NAME = _limine_info_request.response->name;
        BOOTLOADER_VERSION = _limine_info_request.response->version;
    }else{
        BOOTLOADER_NAME = "No Limine Bootloader Info found!";
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

void get_hhdm_info(void){
    if(_hhdm_request.response != NULL){
        HIGHER_HALF_DIRECT_MAP_REVISION = _hhdm_request.response->revision;
        HIGHER_HALF_DIRECT_MAP_OFFSET = _hhdm_request.response->offset;
    }else{
        HIGHER_HALF_DIRECT_MAP_REVISION = 0;
        HIGHER_HALF_DIRECT_MAP_OFFSET = 0;
    }
}


void get_memory_map(void){
    memmap_request = _memmap_request;
    if(_memmap_request.response != NULL){
        uint64_t revision = _memmap_request.response->revision;
        entry_count = _memmap_request.response->entry_count;
        entries = _memmap_request.response->entries;
        for(size_t i=0; i<(size_t) entry_count; i++){
            uint64_t base = entries[i]->base;
            uint64_t length = entries[i]->length;
            uint64_t type = entries[i]->type;

            TOTAL_MEMORY += length;

            if(type == LIMINE_MEMMAP_USABLE){
                USABLE_MEMORY += length;
            }

            if(type == LIMINE_MEMMAP_RESERVED){
                RESERVED_MEMORY += length;
            }

            if(type == LIMINE_MEMMAP_BAD_MEMORY){
                BAD_MEMORY += length;
            }

            if(type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE){
                BOOTLOADER_RECLAIMABLE_MEMORY += length;
            }

            if(type == LIMINE_MEMMAP_ACPI_RECLAIMABLE){
                ACPI_RECLAIMABLE_MEMORY += length;
            }

            if(type == LIMINE_MEMMAP_ACPI_NVS){
                ACPI_NVS_MEMORY += length;
            }


        }
    }
}

void print_memory_map(void) {
    // Check if the memory map response is available
    if (_memmap_request.response == NULL) {
        print("Memory map request failed.\n");
        return;
    }

    print("Kernel memory start address : ");
    print_size_with_units(kernel_placement_address);
    print("\n");
    print("Kernel memory size : ");
    print_size_with_units(kernel_length);
    print("\n");

    print("User memory start address : ");
    print_size_with_units(user_placement_address);
    print("\n");
    print("User memory size : ");
    print_size_with_units(user_length);
    print("\n");

    print("Start address of storing frames used or unused data : ");
    print_hex((uint64_t)frames);
    print("\n");
    print("Total frames : ");
    print_dec(nframes);
    print("\n");


    print("Memory Map Entries : ");
    print_dec(entry_count);
    print("\n");

    print("Memory Map:\n");
    for (uint64_t i = 0; i < entry_count; i++) {
        struct limine_memmap_entry *entry = entries[i];
        print("\tRegion ");
        print_dec(i);

        print(": Base = ");
        print_hex(entry->base);
        print(" [");
        print_size_with_units(entry->base);
        print("] ");

        print(", Length = ");
        print_hex(entry->length);
        print(" [");
        print_size_with_units(entry->length);
        print("] ");


        // Check the type and print it
        switch (entry->type) {
            case LIMINE_MEMMAP_USABLE: // 0, This memory is available for the operating system to use freely. It is not reserved for any specific purpose by hardware or firmware.
                print(" Usable.\n");
                break;
            case LIMINE_MEMMAP_RESERVED: // 1, This memory is reserved and should not be modified. It might be used by firmware, hardware, or other components that the OS cannot interfere with.
                print(" Reserved, not be modified!\n");
                break;
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE: // 2, Memory used by the ACPI (Advanced Configuration and Power Interface) tables. After the ACPI tables have been parsed and used, this memory can be reclaimed and repurposed by the operating system.
                print(" ACPI Reclaimable, can be usable.\n");
                break;
            case LIMINE_MEMMAP_ACPI_NVS :   // 3, Non-Volatile Storage (NVS) memory used by the ACPI for storing runtime configuration and state. This memory must not be modified or reclaimed as it is needed by the system during runtime.
                print(" ACPI NVS, not be modified!\n");
                break;
            case LIMINE_MEMMAP_BAD_MEMORY: // 4, This memory region is marked as bad and unreliable due to detected hardware errors or inconsistencies.
                print(" Bad Memory, not usable!\n");
                break;
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE: // 5, Memory used temporarily by the bootloader. Once the operating system is fully loaded, this memory can be reclaimed and repurposed.
                print(" Bootloader reclaimable, can be usable.\n");
                break;
            case 6: // LIMINE_MEMMAP_EXECUTABLE_AND_MODULES tag not working so i used custom value 6, Memory occupied by the kernel image and modules loaded by the bootloader. This type is typically not directly supported in some Limine versions and might need custom handling.
                    print(" Kernel/Modules, not usable!\n");
                break;
            case LIMINE_MEMMAP_FRAMEBUFFER: // 7, Memory used for the framebuffer, which holds pixel data for the display. The operating system must not overwrite this memory unless it takes control of the display.
                print(" Framebuffer, not be usable!\n");
                break;
            default:
                print(" Unknown Type !!\n"); // An undefined or unrecognized type, often indicative of an implementation issue or an unsupported memory region.
        }
    }
}

void get_kernel_to_virtual_offset(void){
    if(_kernel_address_request.response != NULL){
        KERNEL_ADDRESS_REVISION = _kernel_address_request.response->revision;
        PHYSICAL_BASE = _kernel_address_request.response->physical_base;
        VIRTUAL_BASE = _kernel_address_request.response->virtual_base;
        PHYSICAL_TO_VIRTUAL_OFFSET = VIRTUAL_BASE - PHYSICAL_BASE;
    }else{
        PHYSICAL_TO_VIRTUAL_OFFSET = 0;
    }
}

void get_bootloader_info(void){
    get_firmware_info();
    get_stack_info();
    get_limine_info();
    get_paging_mode_info();
    get_smp_info();
    get_hhdm_info();
    get_memory_map();
    get_kernel_to_virtual_offset();
}



void print_bootloader_info(void){
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

    print("Total Memory : ");
    print_size_with_units(TOTAL_MEMORY);
    print("\n");

    print("Usable Memory : ");
    print_size_with_units(USABLE_MEMORY);
    print("\n");

    print("Reserved Memory : ");
    print_size_with_units(RESERVED_MEMORY);
    print("\n");

    print("Bad Memory : ");
    print_size_with_units(BAD_MEMORY);
    print("\n");

    print("Bootloader Reclaimable Memory : ");
    print_size_with_units(BOOTLOADER_RECLAIMABLE_MEMORY);
    print("\n");

    print("ACPI Reclaimable Memory : ");
    print_size_with_units(ACPI_RECLAIMABLE_MEMORY);
    print("\n");

    print("ACPI NVS Memory : ");
    print_size_with_units(ACPI_NVS_MEMORY);
    print("\n");

    print("Kernel Modules Memory : ");
    print_size_with_units(KERNEL_MODULES_MEMORY);
    print("\n");



    print("HIGHER_HALF_DIRECT_MAP_REVISION : ");
    print_dec(HIGHER_HALF_DIRECT_MAP_REVISION);
    print("\n");

    print("HIGHER_HALF_DIRECT_MAP_OFFSET : ");
    print_hex(HIGHER_HALF_DIRECT_MAP_OFFSET);
    print("\n");

    print("PHYSICAL_TO_VIRTUAL_OFFSET : ");
    print_hex(PHYSICAL_TO_VIRTUAL_OFFSET);
    print("\n");
}









