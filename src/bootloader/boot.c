
/*
Currently I am using Limine Bootloader, so I will use the Limine Bootloader protocol 
to get the bootloader information. The bootloader information is stored in the 
bootloader_info_request struct. The bootloader_info_request struct is defined in 
the kernel.c file. The bootloader_info_request struct contains the bootloader name 
and version. The bootloader name and version are stored in the name and version  
fields of the bootloader_info_request struct.

In future I will implement a custom bootloader and will use that .

Reference: https://github.com/limine-bootloader/limine/blob/v8.x/PROTOCOL.md#kernel-address-feature
*/


#include "boot.h"


__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(2);

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;




// Get Firmware info
__attribute__((used, section(".requests")))
static volatile struct limine_firmware_type_request firmware_type_request = {
    .id = LIMINE_FIRMWARE_TYPE_REQUEST,
    .revision = 2
};

char *FIRMWARE_TYPE;

void get_firmware_info(){
    if(firmware_type_request.response != NULL){
        uint64_t firmware_type = firmware_type_request.response->firmware_type;
        if(firmware_type == LIMINE_FIRMWARE_TYPE_X86BIOS){
            FIRMWARE_TYPE = "X86BIOS";
        }else if(firmware_type == LIMINE_FIRMWARE_TYPE_UEFI32){
            FIRMWARE_TYPE = "UEFI32";
        }else if(firmware_type == LIMINE_FIRMWARE_TYPE_UEFI64){
            FIRMWARE_TYPE = "UEFI64";
        } 
    }else{
        FIRMWARE_TYPE = "No firmware type found!";
        print("No firmware type found!\n");
    }
}

void print_firmware_info(){
    if(firmware_type_request.response != NULL){
        print("Firmware type: ");
        print(FIRMWARE_TYPE);
        print("\n");
    }else{
        FIRMWARE_TYPE = "No firmware type found!";
        print("No firmware type found!\n");
    }
}




// Multiprocessor info
__attribute__((used, section(".requests")))
static volatile struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 2
};

uint64_t CPU_COUNT;

uint64_t MULTIPROCESSOR_REVISION;
uint64_t MULTIPROCESSOR_OFFSET;

void get_smp_info(){
    if(smp_request.response != NULL){
        // uint64_t revision = smp_request.response->revision;
        // uint64_t flags = smp_request.response->flags;
        // uint64_t bsp_lapic_id = smp_request.response->bsp_lapic_id; // The Local APIC ID of the bootstrap processor.
        CPU_COUNT = smp_request.response->cpu_count; //  How many CPUs are present. It includes the bootstrap processor.
        //struct limine_smp_info **cpus = smp_request.response->cpus; // Pointer to an array of cpu_count pointers to struct limine_smp_info structures.
        //for(size_t i=0;i<(size_t) CPU_COUNT;i++){
            // uint64_t processor_id = cpus[0]->processor_id;
            // uint64_t lapic_id = cpus[0]->lapic_id;
            // uint64_t reserved = cpus[0]->reserved;
            // limine_goto_address goto_address = cpus[0]->goto_address;
            // uint64_t extra_argument = cpus[0]->extra_argument;
        //}
    }else{
        CPU_COUNT = 0;
        print("No SMP info found!\n");
    }
}

void print_smp_info(){
    if(smp_request.response != NULL){
        print("CPU COUNT: ");
        print_dec(CPU_COUNT);
        print("\n");
    }else{
        print("No SMP info found!\n");
    }
}





// Get Paging info

__attribute__((used, section(".requests")))
static volatile struct limine_paging_mode_request paging_mode_request = {
    .id = LIMINE_PAGING_MODE_REQUEST,
    .revision = 2
};


char *LIMINE_PAGING_MODE;

void get_paging_mode_info(){
    if(paging_mode_request.response != NULL){
        uint64_t mode = paging_mode_request.response->mode;
        if(mode == LIMINE_PAGING_MODE_X86_64_4LVL){
            LIMINE_PAGING_MODE = "Limine Paging mode x86_64 4 Level";
        } else if(mode == LIMINE_PAGING_MODE_X86_64_5LVL){
            LIMINE_PAGING_MODE = "Limine Paging mode x86_64 5 Level";
        }
    }else{
        LIMINE_PAGING_MODE = "No Paging mode found!";
        print("No Paging mode found!\n");
    }
}

void print_paging_mode_info(){
     if(paging_mode_request.response != NULL){
        print("Limine Paging Mode : ");
        print(LIMINE_PAGING_MODE);
        print("\n");
    }else{
        LIMINE_PAGING_MODE = "No Paging mode found!";
        print("No Paging mode found!\n");
    }
}





// Get  Limine Bootloader name, version etc 
__attribute__((used, section(".requests")))
static volatile struct limine_bootloader_info_request limine_info_request = {
    .id = LIMINE_BOOTLOADER_INFO_REQUEST,
    .revision = 2
};

char *BOOTLOADER_NAME;
char *BOOTLOADER_VERSION;

void get_limine_info(){
    if(limine_info_request.response != NULL){
        // uint64_t revision = limine_info_request.response->revision;
        BOOTLOADER_NAME = limine_info_request.response->name;
        BOOTLOADER_VERSION = limine_info_request.response->version;
    }else{
        BOOTLOADER_NAME = "No Limine Bootloader Info found!";
        BOOTLOADER_VERSION = NULL;
        print("No Limine Bootloader Info found!\n");
    }
}

void print_limine_info(){
    if(limine_info_request.response != NULL){
        print("Bootloader Name : ");
        print(BOOTLOADER_NAME);
        print("\n");
        print("Bootloader Version : ");
        print(BOOTLOADER_VERSION);
        print("\n");

    }else{
        print("No Limine Bootloader Info found!\n");
    }
}





// Get Stack size info
__attribute__((used, section(".requests")))
static volatile struct limine_stack_size_request stack_size_request = {
    .id = LIMINE_STACK_SIZE_REQUEST,
    .revision = 2,
    .stack_size = 16384
};

uint64_t STACK_SIZE;

void get_stack_info(){
    if(stack_size_request.response != NULL){
        STACK_SIZE = stack_size_request.stack_size;
    }else{
        STACK_SIZE = 0;
        print("No stack size found!\n");
    }
}

void print_stack_info(){
    if(stack_size_request.response != NULL){
        print("Stack Size : ");
        print_dec(STACK_SIZE);
        print("\n");
    }else{
        print("No stack size found!\n");
    }
}

// Get memory info
__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 2
};

uint64_t KMEM_UP_BASE;
uint64_t KMEM_LOW_BASE;
uint64_t KMEM_LENGTH;

uint64_t UMEM_LOW_BASE;
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

        // place kernel into higher  usable memory space
        KMEM_UP_BASE = entries[entry_ids[3]]->base;
        KMEM_LENGTH = entries[entry_ids[3]]->length;
        KMEM_LOW_BASE = KMEM_UP_BASE - KMEM_LENGTH;

        // place user into lower half usable memory space
        UMEM_LOW_BASE = entries[entry_ids[1]]->base;
        UMEM_LENGTH = entries[entry_ids[1]]->length;
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
    .revision = 2
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

        // V_KMEM_UP_BASE = VIRTUAL_BASE + 0x40000000 - 0x322000; // 1GB
        // V_KMEM_UP_BASE = VIRTUAL_BASE + KMEM_LENGTH;
        V_KMEM_UP_BASE = 0xFFFFFFFFFFFFFFFF;

        KMEM_LOW_BASE = V_KMEM_LOW_BASE - PHYSICAL_TO_VIRTUAL_OFFSET;
        KMEM_UP_BASE = V_KMEM_UP_BASE - PHYSICAL_TO_VIRTUAL_OFFSET;

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
    .revision = 2
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



// Get Module info
__attribute__((used, section(".requests")))
static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 2
};


void get_kernel_modules_info(){
    if(module_request.response != NULL){
        //uint64_t revision = module_request.response->revision;
        //uint64_t module_count = module_request.response->module_count;
        // struct limine_file **modules = module_request.response->modules;
    }else{
        print("No kernel modules found!\n");
    }
}

void print_kernel_modules_info(){
    if(module_request.response != NULL){
        // uint64_t revision = module_request.response->revision;
        uint64_t module_count = module_request.response->module_count;
        struct limine_file **modules = module_request.response->modules;
        for(size_t i=0;i<(size_t) module_count;i++){
            // uint64_t revision = modules[i]->revision;
            void *address = modules[i]->address;
            uint64_t size = modules[i]->size;
            char *path = modules[i]->path;
            // char *cmdline = modules[i]->cmdline;
            // uint32_t media_type = modules[i]->media_type;
            // uint32_t unused = modules[i]->unused;
            // uint32_t tftp_ip = modules[i]->tftp_ip;
            // uint32_t tftp_port = modules[i]->tftp_port;
            uint32_t partition_index = modules[i]->partition_index;
            uint32_t mbr_disk_id = modules[i]->mbr_disk_id;
            // struct limine_uuid gpt_disk_uuid = modules[i]->gpt_disk_uuid;
            // struct limine_uuid gpt_part_uuid = modules[i]->gpt_part_uuid;
            // struct limine_uuid part_uuid = modules[i]->part_uuid;

            print("Module Path : ");
            print(path);
            print("\n");

            print("Module Address : ");
            print_hex((uint64_t) address);
            print("\n");

            print("Module Size : ");
            print_size_with_units(size);
            print("\n");

            print("Module Media Type : ");
            // print_dec(media_type);
            print("\n");

            print("Module Partition Index : ");
            print_dec(partition_index);
            print("\n");

            print("Module MBR Disk ID : ");
            print_dec(mbr_disk_id);
            print("\n");

            
        }
    }else{
        print("No kernel modules found!\n");
    }
}






// Get RSDP info
__attribute__((used, section(".requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 2
};

// void parse_xsdt(struct XSDT *xsdt) {
//     uint64_t num_entries = (xsdt->header.length - sizeof(xsdt->header)) / sizeof(uint64_t);
//     for (uint64_t i = 0; i < num_entries; i++) {
//         struct ACPI_SDT_Header *table = (struct ACPI_SDT_Header *) xsdt->entries[i];
//         print("ACPI Table Signature: ");
//         for (int j = 0; j < 4; j++) {
//             putchar(table->signature[j]);
//         }
//         print("\n");
//     }
// }

uint64_t *RSDP_PTR;
uint64_t RSDP_REVISION;
void get_rsdp_info(){
    if(rsdp_request.response != NULL){
        RSDP_REVISION = rsdp_request.response->revision;
        RSDP_PTR = rsdp_request.response->address; 
    }else{
        RSDP_PTR = NULL;
        RSDP_REVISION = 0;
        print("No RSDP info found!\n");
    }
}

void print_rsdp_info(){
    if(rsdp_request.response != NULL){

        struct RSDP *rsdp = (struct RSDP *) rsdp_request.response->address;

        print("RSDP Signature: ");
        for (int i = 0; i < 8; i++) {
            putchar(rsdp->signature[i]);
        }
        print("\n");

        print("RSDP OEMID: ");
        for (int i = 0; i < 6; i++) {
            putchar(rsdp->OEMID[i]);
        }
        print("\n");

        print("RSDP Revision: ");
        print_dec(rsdp->revision);
        print("\n");

        if (rsdp->revision >= 2) {
            print("XSDT Address: ");
            print_hex(rsdp->xsdt_address);
            print("\n");
        } else {
            print("RSDT Address: ");
            print_hex(rsdp->rsdt_address);
            print("\n");
        }
    }else{
        print("No RSDP info found!\n");
    }
}


// CPUID instruction wrapper
static inline void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    __asm__ volatile("cpuid"
                     : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
                     : "a" (leaf), "c" (0));
}

// Get the processor name
void get_processor_name(char *name_buffer) {
    uint32_t eax, ebx, ecx, edx;

    // Leaf 0x80000002
    cpuid(0x80000002, &eax, &ebx, &ecx, &edx);
    memcpy(name_buffer, &eax, 4);
    memcpy(name_buffer + 4, &ebx, 4);
    memcpy(name_buffer + 8, &ecx, 4);
    memcpy(name_buffer + 12, &edx, 4);

    // Leaf 0x80000003
    cpuid(0x80000003, &eax, &ebx, &ecx, &edx);
    memcpy(name_buffer + 16, &eax, 4);
    memcpy(name_buffer + 20, &ebx, 4);
    memcpy(name_buffer + 24, &ecx, 4);
    memcpy(name_buffer + 28, &edx, 4);

    // Leaf 0x80000004
    cpuid(0x80000004, &eax, &ebx, &ecx, &edx);
    memcpy(name_buffer + 32, &eax, 4);
    memcpy(name_buffer + 36, &ebx, 4);
    memcpy(name_buffer + 40, &ecx, 4);
    memcpy(name_buffer + 44, &edx, 4);

    // Null-terminate the string
    name_buffer[48] = '\0';
}

void print_processor_name(char processor_name[49]){
    print("Processor Name: ");
    print(processor_name);
    print("\n");
}




char processor_name[49];
// finuls functions which will actually use in different ways
void get_bootloader_info(){
    get_memory_map();
    get_kernel_to_virtual_offset();
    get_kernel_modules_info();
    get_rsdp_info();
    get_firmware_info();
    get_stack_info();
    get_limine_info();
    get_paging_mode_info();
    get_smp_info();
    get_processor_name(processor_name);
    get_hhdm_info();
    
}

void print_bootloader_info(void){
    print_kernel_modules_info();
    print_rsdp_info();
    print_firmware_info();
    print_stack_info();
    print_limine_info();
    print_paging_mode_info();
    print_smp_info();
    print_processor_name(processor_name); 
    print_hhdm_info();
    print_memory_map();
    print_kernel_to_virtual_offset();

}
