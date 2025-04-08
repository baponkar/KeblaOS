
#include "../../../limine-8.6.0/limine.h"   // bootloader info
#include "../driver/vga/vga_term.h"
#include "../util/util.h" 

#include "detect_memory.h"


__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

// Get memory info
__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 3
};


uint64_t KMEM_UP_BASE;
uint64_t KMEM_LOW_BASE;
uint64_t KMEM_LENGTH;

uint64_t UMEM_LOW_BASE = 0x1000;
uint64_t UMEM_UP_BASE;
uint64_t UMEM_LENGTH;

uint64_t TOTAL_MEMORY;
uint64_t USABLE_MEMORY;
uint64_t RESERVED_MEMORY;
uint64_t BAD_MEMORY;
uint64_t BOOTLOADER_RECLAIMABLE_MEMORY;
uint64_t ACPI_RECLAIMABLE_MEMORY;
uint64_t ACPI_NVS_MEMORY;
uint64_t FRAMEBUFFER_MEMORY;
uint64_t KERNEL_MODULES_MEMORY;
uint64_t UNKNOWN_MEMORY;

uint64_t entry_count;
struct limine_memmap_entry **entries;

void get_memory_map(){ 
    size_t entry_ids[4];    // This array will store the index of the first 4 usable memory regions
    for (size_t i = 0; i < 4; i++)
    {
        entry_ids[i] = 0;   // initialise the entry_ids array with 0
    }
    size_t tmp = 0; // temporary variable to store the index of the first 4 usable memory regions

    if(memmap_request.response != NULL){

        // uint64_t revision = memmap_request.response->revision;
        entry_count = memmap_request.response->entry_count;
        entries = memmap_request.response->entries;

        for(size_t i=0; i<(size_t) entry_count; i++){
            uint64_t base = entries[i]->base;
            uint64_t length = entries[i]->length;
            uint64_t type = entries[i]->type;

            TOTAL_MEMORY += length;

            if(type == LIMINE_MEMMAP_USABLE){
                entry_ids[tmp] = i; // store the index of the first 4 usable memory regions
                tmp++;
                USABLE_MEMORY += length;
            }else if(type == LIMINE_MEMMAP_RESERVED){
                RESERVED_MEMORY += length;
            }else if(type == LIMINE_MEMMAP_BAD_MEMORY){
                BAD_MEMORY += length;
            }else if(type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE){
                BOOTLOADER_RECLAIMABLE_MEMORY += length;
            }else if(type == LIMINE_MEMMAP_ACPI_RECLAIMABLE){
                ACPI_RECLAIMABLE_MEMORY += length;
            }else if(type == LIMINE_MEMMAP_ACPI_NVS){
                ACPI_NVS_MEMORY += length;
            }else{
                UNKNOWN_MEMORY += length;
            }
        }

        if(entry_ids[3] == 0){
            entry_ids[3] = entry_ids[2];
        }

        // Placing lower physical memory for kernel space although Higher half virtual address would be use for kernel
        KMEM_UP_BASE = entries[entry_ids[1]]->base;
        KMEM_LENGTH = entries[entry_ids[1]]->length;
        KMEM_LOW_BASE = KMEM_UP_BASE - KMEM_LENGTH;

        // Placing upper physical memory for user space although Lower half  virtual address would be use for user
        // UMEM_LOW_BASE = entries[entry_ids[3]]->base;
        UMEM_LENGTH = entries[entry_ids[3]]->length;
        UMEM_UP_BASE = UMEM_LOW_BASE + UMEM_LENGTH;
    }else{
        print("Memory map request failed.\n");
    }
}



void print_memory_map() {
    
    // Check if the memory map response is available
    if (memmap_request.response == NULL) {
        print("Memory map request failed.\n");
        return;
    }

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

    print("Unknown Memory : ");
    print_size_with_units(UNKNOWN_MEMORY);
    print("\n");

    print("Kernel Modules Memory : ");
    print_size_with_units(KERNEL_MODULES_MEMORY);
    print("\n");

    print("Kernel memory start address : ");
    print_size_with_units(KMEM_LOW_BASE);
    print("\n");
    print("Kernel memory end address : ");
    print_size_with_units(KMEM_UP_BASE);
    print("\n");
    print("Kernel memory size : ");
    print_size_with_units(KMEM_LENGTH);
    print("\n");

    print("User memory start address : ");
    print_size_with_units(UMEM_LOW_BASE);
    print("\n");
    print("User memory end address : ");
    print_size_with_units(UMEM_UP_BASE);
    print("\n");
    print("User memory size : ");
    print_size_with_units(UMEM_LENGTH);
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



// Get Virtual to Physical offset
__attribute__((used, section(".requests")))
static volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 3
};

uint64_t KERNEL_ADDRESS_REVISION;
uint64_t PHYSICAL_BASE;
uint64_t VIRTUAL_BASE;
uint64_t PHYSICAL_TO_VIRTUAL_OFFSET;

uint64_t V_KMEM_LOW_BASE;
uint64_t V_KMEM_UP_BASE;

uint64_t V_UMEM_LOW_BASE;
uint64_t V_UMEM_UP_BASE;

void get_kernel_to_virtual_offset(){
    if(kernel_address_request.response != NULL){
        KERNEL_ADDRESS_REVISION = kernel_address_request.response->revision;
        PHYSICAL_BASE = kernel_address_request.response->physical_base;
        VIRTUAL_BASE = kernel_address_request.response->virtual_base;
        PHYSICAL_TO_VIRTUAL_OFFSET = VIRTUAL_BASE - PHYSICAL_BASE;

        // V_KMEM_LOW_BASE = VIRTUAL_BASE;
        V_KMEM_LOW_BASE = 0xFFFFFFFF80322000;
        // V_KMEM_LOW_BASE = 0xFFFF800000000000;

        // V_KMEM_UP_BASE = VIRTUAL_BASE + 0x40000000 - 0x322000; // 1GB
        // V_KMEM_UP_BASE = VIRTUAL_BASE + KMEM_LENGTH;
        V_KMEM_UP_BASE = 0xFFFFFFFFFFFFFFFF;

        KMEM_LOW_BASE = V_KMEM_LOW_BASE - PHYSICAL_TO_VIRTUAL_OFFSET;
        KMEM_UP_BASE = V_KMEM_UP_BASE - PHYSICAL_TO_VIRTUAL_OFFSET;

        V_UMEM_LOW_BASE = 0x1000; // I want to start users space memory from 0x1000 instead of 0x0
        V_UMEM_UP_BASE = 0x00007FFFFFFFFFFF;

    }else{
        PHYSICAL_TO_VIRTUAL_OFFSET = 0;
        print("No kernel to virtual offset found!\n");
    }
}

void print_kernel_to_virtual_offset(){
    if(kernel_address_request.response != NULL){
        print("Kernel address revision: ");
        print_dec(KERNEL_ADDRESS_REVISION);
        print("\n");
        print("Physical Base : ");
        print_hex(PHYSICAL_BASE);
        print("\n");
        print("Virtual Base : ");
        print_hex(VIRTUAL_BASE);
        print("\n");
        print("Physical address to Virtual address Offset : ");
        print_hex(PHYSICAL_TO_VIRTUAL_OFFSET);
        print("\n");
    }else{
        print("No kernel to virtual offset found!\n");
    }
}


// Get Higher half direct map offset 
__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 3
};

uint64_t HHDM_REVISION;
uint64_t HHDM_OFFSET;

void get_hhdm_info(){
    if(hhdm_request.response != NULL){
        HHDM_REVISION = hhdm_request.response->revision;
        HHDM_OFFSET = hhdm_request.response->offset;
    }else{
        HHDM_REVISION = 0;
        HHDM_OFFSET = 0;
        print("No Higher Half Direct Map info found!\n");
    }
}

void print_hhdm_info(){
    if(hhdm_request.response != NULL){
        print("Higher half direct map revision : ");
        print_dec(HHDM_REVISION);
        print("\n");
        print("Higher Half Direct Map Offset : ");
        print_hex(HHDM_OFFSET);
        print("\n");
    }else{
        print("No Higher Half Direct Map info found!\n");
    }
}

void get_memory_info(){
    get_memory_map();
    get_kernel_to_virtual_offset();
    get_hhdm_info();
}
