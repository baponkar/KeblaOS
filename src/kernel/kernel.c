<<<<<<< HEAD

/*
Kernel.c
Build Date  : 05/12/2024
Description :
Reference   : 
              https://github.com/limine-bootloader/limine/blob/v8.x/PROTOCOL.md#kernel-address-feature
              https://wiki.osdev.org/Limine
              https://github.com/limine-bootloader/limine-c-template
              https://wiki.osdev.org/Limine_Bare_Bones

*/

#include "kernel.h"

// My plan is set value of the below variable by bootloader data

// Note from Michael Petch   0x1000000 is not 1Mib! Extra 0??
//                           0x0050000 is not 20KiB! Extra 0??
// Are you sure about these value. Michael agrees that your PMM
// needs to be tied to the physical memory that Limine tells you is
// available to be used by your kernel.
uint64_t placement_address = 0x1000000;  // 1 MB
uint64_t mem_end_address = 0x1050000; // 1MB + 20 KB


char *OS_NAME = "KeblaOS";
char *OS_VERSION = "0.9";
char *BUILD_DATE = "14/12/2024";

char *FIRMWARE_TYPE;

uint32_t *FRAMEBUFFER_PTR;   // Note: we assume the framebuffer model is RGB with 32-bit pixels.
size_t FRAMEBUFFER_WIDTH;
size_t FRAMEBUFFER_HEIGHT;

struct limine_memmap_response* MEMMAP_INFO_PTR;

uint64_t VIRTUAL_BASE;
uint64_t PHYSICAL_BASE;
uint64_t VIRTUAL_TO_PHYSICAL_OFFSET;

uint64_t CPU_COUNT;

char *BOOTLOADER_NAME;
char *BOOTLOADER_VERSION;

uint64_t STACK_SIZE;

uint64_t HIGHER_HALF_DIRECT_MAP_REVISION;
uint64_t HIGHER_HALF_DIRECT_MAP_OFFSET;

uint64_t MULTIPROCESSOR_REVISION;
uint64_t MULTIPROCESSOR_OFFSET;

char *LIMINE_PAGING_MODE;

// Set the base revision to 3, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(0);


__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};


__attribute__((used, section(".requests")))
static volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};


__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};


__attribute__((used, section(".requests")))
static volatile struct limine_firmware_type_request firmware_type_request = {
    .id = LIMINE_FIRMWARE_TYPE_REQUEST,
    .revision = 0
};

// Multiprocessor info

__attribute__((used, section(".requests")))
static volatile struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0
};


__attribute__((used, section(".requests")))
static volatile struct limine_paging_mode_request paging_mode_request = {
    .id = LIMINE_PAGING_MODE_REQUEST,
    .revision = 0
};


__attribute__((used, section(".requests")))
static volatile struct limine_bootloader_info_request bootloader_info_request = {
    .id = LIMINE_BOOTLOADER_INFO_REQUEST,
    .revision = 0
};


__attribute__((used, section(".requests")))
static volatile struct limine_stack_size_request stack_size_request = {
    .id = LIMINE_STACK_SIZE_REQUEST,
    .revision = 0,
    .stack_size = 8192
};


__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};


__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;


void kmain(void){
    get_system_info();

<<<<<<< HEAD
    // print_bootloader_info();
=======
    print("KeblaOS - 0.8\n");
    print_bootloader_info();
>>>>>>> 90f9877f1f464214e589661cd94902285944ad34

    init_gdt();
    // check_gdt();

    init_idt();
    test_interrupt();

    // initialise_paging();

    // init_timer();
    // initKeyboard();

    

    // Test paging
    // test_paging();

    hcf();
}




void get_system_info(){
    get_framebuffer_info();
    vga_init();
    get_firmware_info();
    get_hhdm_info();
    get_stack_info();
    get_bootloader_info();
    get_paging_mode_info();
    get_smp_info();
    get_vir_to_phy_offset();
}


void print_bootloader_info(){
    print("BUILD_DATE : ");
    print(BUILD_DATE);
    print("\n");

    print("Bootloader : ");
    print(BOOTLOADER_NAME);
    print(" ");
    print(BOOTLOADER_VERSION);
    print("\n");

    print_memory_map();

    print("Framebuffer Resolution : ");
    print_dec(FRAMEBUFFER_WIDTH);
    print("x");
    print_dec(FRAMEBUFFER_HEIGHT);
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

    print("VIRTUAL_BASE : ");
    print_hex(VIRTUAL_BASE);
    print("\n");
    print("PHYSICAL_BASE : ");
    print_hex(PHYSICAL_BASE);
    print("\n");
    print("VIRTUAL_TO_PHYSICAL_OFFSET : ");
    print_hex(VIRTUAL_TO_PHYSICAL_OFFSET);
    print("\n");
}


// Following functions will get information from limine bootloader
void get_framebuffer_info(void){

    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    // Ensure the framebuffer is initialized
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer
    struct limine_framebuffer* framebuffer = framebuffer_request.response->framebuffers[0];
    FRAMEBUFFER_PTR = (uint32_t*) framebuffer->address;

    FRAMEBUFFER_WIDTH = framebuffer->width;
    FRAMEBUFFER_HEIGHT = framebuffer->height;
}


void get_firmware_info(void){
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
    }
}

void get_hhdm_info(void){
    if(hhdm_request.response != NULL){
        HIGHER_HALF_DIRECT_MAP_REVISION = hhdm_request.response->revision;
        HIGHER_HALF_DIRECT_MAP_OFFSET = hhdm_request.response->offset;
    }else{
        HIGHER_HALF_DIRECT_MAP_REVISION = 0;
        HIGHER_HALF_DIRECT_MAP_OFFSET = 0;
    }
}

void get_stack_info(void){
    if(stack_size_request.response != NULL){
        STACK_SIZE = stack_size_request.stack_size;
    }else{
        STACK_SIZE = 0;
    }
}

void get_bootloader_info(void){
    if(bootloader_info_request.response != NULL){
        uint64_t revision = bootloader_info_request.response->revision;
        BOOTLOADER_NAME = bootloader_info_request.response->name;
        BOOTLOADER_VERSION = bootloader_info_request.response->version;
    }else{
        BOOTLOADER_NAME = "No Bootloader Info found!";
        BOOTLOADER_VERSION = NULL;
    }
}


void get_paging_mode_info(void){
    if(paging_mode_request.response != NULL){
        uint64_t mode = paging_mode_request.response->mode;
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
    if(smp_request.response != NULL){
        uint64_t revision = smp_request.response->revision;
        uint64_t flags = smp_request.response->flags;
        uint64_t bsp_lapic_id = smp_request.response->bsp_lapic_id; // The Local APIC ID of the bootstrap processor.
        CPU_COUNT = smp_request.response->cpu_count; //  How many CPUs are present. It includes the bootstrap processor.
        struct limine_smp_info **cpus = smp_request.response->cpus; // Pointer to an array of cpu_count pointers to struct limine_smp_info structures.
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

void get_vir_to_phy_offset(void){
     if (kernel_address_request.response != NULL) {
        PHYSICAL_BASE = kernel_address_request.response->physical_base;
        VIRTUAL_BASE = kernel_address_request.response->virtual_base;

        // Calculate the offset between virtual and physical addresses.
        VIRTUAL_TO_PHYSICAL_OFFSET = VIRTUAL_BASE - PHYSICAL_BASE;

    }else{
        PHYSICAL_BASE = 0;
        VIRTUAL_BASE = 0;
        VIRTUAL_TO_PHYSICAL_OFFSET = 0;
    }
}

void print_memory_map(void) {
    // Check if the memory map response is available
    if (memmap_request.response == NULL) {
        print("Memory map request failed.\n");
        return;
    }

    uint64_t entry_count = memmap_request.response->entry_count;
    struct limine_memmap_entry **entries = memmap_request.response->entries;

    print("Memory Map:\n");
    for (uint64_t i = 0; i < entry_count; i++) {
        struct limine_memmap_entry *entry = entries[i];
        print("\tRegion ");
        print_hex(i);
        print(": Base = ");
        print_hex(entry->base);
        print(", Length = ");
        print_hex(entry->length);

        // Check the type and print it
        switch (entry->type) {
            case 0x0:
                print(" (Usable)\n");
                break;
            case 0x1:
                print(" (Reserved)\n");
                break;
            case 0x2:
                print(" (ACPI Reclaimable)\n");
                break;
            case 0x3:
                print(" (ACPI NVS)\n");
                break;
            case 0x4:
                print(" (Bad Memory)\n");
                break;
            case 0x5:
                print(" Bootloader reclaimable\n");
                break;
            case 0x6:
                print(" Kernel/Modules\n");
                break;
            case 0x7:
                print(" Framebuffer\n");
                break;
            default:
                print(" (Unknown Type !!)\n");
        }
    }
}



=======

/*
Kernel.c
Build Date  : 05/12/2024
Description :
Reference   : 
              https://github.com/limine-bootloader/limine/blob/v8.x/PROTOCOL.md#kernel-address-feature
              https://wiki.osdev.org/Limine
              https://github.com/limine-bootloader/limine-c-template
              https://wiki.osdev.org/Limine_Bare_Bones

*/

#include "kernel.h"

// My plan is set value of the below variable by bootloader data

// Note from Michael Petch   0x1000000 is not 1Mib! Extra 0??
//                           0x0050000 is not 20KiB! Extra 0??
// Are you sure about these value. Michael agrees that your PMM
// needs to be tied to the physical memory that Limine tells you is
// available to be used by your kernel.
uint64_t placement_address = 0x1000000;  // 1 MB
uint64_t mem_end_address = 0x1050000; // 1MB + 20 KB


char *OS_NAME = "KeblaOS";
char *OS_VERSION = "0.8";
char *BUILD_DATE = "06/12/2024";

char *FIRMWARE_TYPE;

uint32_t *FRAMEBUFFER_PTR;   // Note: we assume the framebuffer model is RGB with 32-bit pixels.
size_t FRAMEBUFFER_WIDTH;
size_t FRAMEBUFFER_HEIGHT;

struct limine_memmap_response* MEMMAP_INFO_PTR;

uint64_t VIRTUAL_BASE;
uint64_t PHYSICAL_BASE;
uint64_t VIRTUAL_TO_PHYSICAL_OFFSET;

uint64_t CPU_COUNT;

char *BOOTLOADER_NAME;
char *BOOTLOADER_VERSION;

uint64_t STACK_SIZE;

uint64_t HIGHER_HALF_DIRECT_MAP_REVISION;
uint64_t HIGHER_HALF_DIRECT_MAP_OFFSET;

uint64_t MULTIPROCESSOR_REVISION;
uint64_t MULTIPROCESSOR_OFFSET;

char *LIMINE_PAGING_MODE;

// Set the base revision to 3, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(0);


__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};


__attribute__((used, section(".requests")))
static volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};


__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};


__attribute__((used, section(".requests")))
static volatile struct limine_firmware_type_request firmware_type_request = {
    .id = LIMINE_FIRMWARE_TYPE_REQUEST,
    .revision = 0
};

// Multiprocessor info

__attribute__((used, section(".requests")))
static volatile struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0
};


__attribute__((used, section(".requests")))
static volatile struct limine_paging_mode_request paging_mode_request = {
    .id = LIMINE_PAGING_MODE_REQUEST,
    .revision = 0
};


__attribute__((used, section(".requests")))
static volatile struct limine_bootloader_info_request bootloader_info_request = {
    .id = LIMINE_BOOTLOADER_INFO_REQUEST,
    .revision = 0
};


__attribute__((used, section(".requests")))
static volatile struct limine_stack_size_request stack_size_request = {
    .id = LIMINE_STACK_SIZE_REQUEST,
    .revision = 0,
    .stack_size = 8192
};


__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};


__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;


void kmain(void){
    get_system_info();

    print("KeblaOS - 0.8\n");
    print_bootloader_info();

    init_gdt();
    // check_gdt();

    init_idt();
    // check_idt();

    // init_timer();
    initKeyboard();

    initialise_paging();

    // Test paging
    // test_paging();

    hcf();
}


// Halt and catch fire function.
void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

void get_system_info(){
    get_framebuffer_info();
    vga_init();
    get_firmware_info();
    get_hhdm_info();
    get_stack_info();
    get_bootloader_info();
    get_paging_mode_info();
    get_smp_info();
    get_vir_to_phy_offset();
}

void print_bootloader_info(){
    print("BUILD_DATE : ");
    print(BUILD_DATE);
    print("\n");

    print("Bootloader : ");
    print(BOOTLOADER_NAME);
    print(" ");
    print(BOOTLOADER_VERSION);
    print("\n");

    print_memory_map();

    print("Framebuffer Resolution : ");
    print_dec(FRAMEBUFFER_WIDTH);
    print("x");
    print_dec(FRAMEBUFFER_HEIGHT);
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

    print("VIRTUAL_BASE : ");
    print_hex(VIRTUAL_BASE);
    print("\n");
    print("PHYSICAL_BASE : ");
    print_hex(PHYSICAL_BASE);
    print("\n");
    print("VIRTUAL_TO_PHYSICAL_OFFSET : ");
    print_hex(VIRTUAL_TO_PHYSICAL_OFFSET);
    print("\n");
}


// Following functions will get information from limine bootloader
void get_framebuffer_info(void){

    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    // Ensure the framebuffer is initialized
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer
    struct limine_framebuffer* framebuffer = framebuffer_request.response->framebuffers[0];
    FRAMEBUFFER_PTR = (uint32_t*) framebuffer->address;

    FRAMEBUFFER_WIDTH = framebuffer->width;
    FRAMEBUFFER_HEIGHT = framebuffer->height;
}

void get_firmware_info(void){
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
    }
}

void get_hhdm_info(void){
    if(hhdm_request.response != NULL){
        HIGHER_HALF_DIRECT_MAP_REVISION = hhdm_request.response->revision;
        HIGHER_HALF_DIRECT_MAP_OFFSET = hhdm_request.response->offset;
    }else{
        HIGHER_HALF_DIRECT_MAP_REVISION = 0;
        HIGHER_HALF_DIRECT_MAP_OFFSET = 0;
    }
}

void get_stack_info(void){
    if(stack_size_request.response != NULL){
        STACK_SIZE = stack_size_request.stack_size;
    }else{
        STACK_SIZE = 0;
    }
}

void get_bootloader_info(void){
    if(bootloader_info_request.response != NULL){
        uint64_t revision = bootloader_info_request.response->revision;
        BOOTLOADER_NAME = bootloader_info_request.response->name;
        BOOTLOADER_VERSION = bootloader_info_request.response->version;
    }else{
        BOOTLOADER_NAME = "No Bootloader Info found!";
        BOOTLOADER_VERSION = NULL;
    }
}


void get_paging_mode_info(void){
    if(paging_mode_request.response != NULL){
        uint64_t mode = paging_mode_request.response->mode;
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
    if(smp_request.response != NULL){
        uint64_t revision = smp_request.response->revision;
        uint64_t flags = smp_request.response->flags;
        uint64_t bsp_lapic_id = smp_request.response->bsp_lapic_id; // The Local APIC ID of the bootstrap processor.
        CPU_COUNT = smp_request.response->cpu_count; //  How many CPUs are present. It includes the bootstrap processor.
        struct limine_smp_info **cpus = smp_request.response->cpus; // Pointer to an array of cpu_count pointers to struct limine_smp_info structures.
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

void get_vir_to_phy_offset(void){
     if (kernel_address_request.response != NULL) {
        PHYSICAL_BASE = kernel_address_request.response->physical_base;
        VIRTUAL_BASE = kernel_address_request.response->virtual_base;

        // Calculate the offset between virtual and physical addresses.
        VIRTUAL_TO_PHYSICAL_OFFSET = VIRTUAL_BASE - PHYSICAL_BASE;

    }else{
        PHYSICAL_BASE = 0;
        VIRTUAL_BASE = 0;
        VIRTUAL_TO_PHYSICAL_OFFSET = 0;
    }
}

void print_memory_map(void) {
    // Check if the memory map response is available
    if (memmap_request.response == NULL) {
        print("Memory map request failed.\n");
        return;
    }

    uint64_t entry_count = memmap_request.response->entry_count;
    struct limine_memmap_entry **entries = memmap_request.response->entries;

    print("Memory Map:\n");
    for (uint64_t i = 0; i < entry_count; i++) {
        struct limine_memmap_entry *entry = entries[i];
        print("\tRegion ");
        print_hex(i);
        print(": Base = ");
        print_hex(entry->base);
        print(", Length = ");
        print_hex(entry->length);

        // Check the type and print it
        switch (entry->type) {
            case 0x0:
                print(" (Usable)\n");
                break;
            case 0x1:
                print(" (Reserved)\n");
                break;
            case 0x2:
                print(" (ACPI Reclaimable)\n");
                break;
            case 0x3:
                print(" (ACPI NVS)\n");
                break;
            case 0x4:
                print(" (Bad Memory)\n");
                break;
            case 0x5:
                print(" Bootloader reclaimable\n");
                break;
            case 0x6:
                print(" Kernel/Modules\n");
                break;
            case 0x7:
                print(" Framebuffer\n");
                break;
            default:
                print(" (Unknown Type !!)\n");
        }
    }
}



>>>>>>> 90f9877f1f464214e589661cd94902285944ad34
