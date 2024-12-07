#include "kernel.h"

// Below variables are used in paging and pmm
uint64_t placement_address = 0x100000;  // 1 MB 
uint64_t mem_end_address = 0x104000; // 1.4 MB

uint32_t *fb_ptr = NULL;

size_t SCREEN_WIDTH;
size_t SCREEN_HEIGHT;

size_t MIN_LINE_NO;
size_t MAX_LINE_NO;

size_t MIN_COLUMN_NO;
size_t MAX_COLUMN_NO;

struct limine_memmap_response* memmap_info;

__attribute__((used, section(".requests")))
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

void kmain(void) {
    get_framebuffer_info();
    init_gdt();
    init_idt();

    init_timer();
    initKeyboard();

    // Checking Paging
    disable_interrupts();
    initialise_paging();
    enable_interrupts();

    hcf();
}

// Halt and catch fire function.
void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

void get_framebuffer_info(){
    // Ensure the framebuffer is initialized
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer
    struct limine_framebuffer* framebuffer = framebuffer_request.response->framebuffers[0];
    fb_ptr = (uint32_t*) framebuffer->address;

    uint64_t mode_count = framebuffer->mode_count;
    struct limine_video_mode **modes = framebuffer->modes;

    SCREEN_WIDTH = framebuffer->width;   // 1024
    SCREEN_HEIGHT = framebuffer->height; // 768

    MIN_LINE_NO = 0;
    MAX_LINE_NO = (SCREEN_HEIGHT / (DEFAULT_FONT_HEIGHT + DEFAULT_TEXT_LINE_GAP)) - 1;

    MIN_COLUMN_NO = 0;
    MAX_COLUMN_NO = (SCREEN_WIDTH / DEFAULT_FONT_WIDTH) - 1;

    // print("\nFramebuffer Info:");
    // print("\nWidth :");
    // print_dec(SCREEN_WIDTH);
    // print("\nHeight : ");
    // print_dec(SCREEN_HEIGHT);
    // print("\nMode Count : ");
    // print_dec(mode_count);

    // for(size_t i = 0; i<mode_count; i++){
    //     struct limine_video_mode *mode = modes[i];
    //     if (mode == NULL) {
    //         printf("Video mode #%u is NULL.\n", i);
    //         continue;
    //     }
    //     printf("\nVideo Mode %d:\n", i);
    //     printf("  Resolution: %ux%u\n", mode->width, mode->height);
    //     printf("  Pitch: %u bytes\n", mode->pitch);
    //     printf("  Bits per Pixel (BPP): %u\n", mode->bpp);
    //     printf("  Memory Model: %u\n", mode->memory_model);
    //     printf("  Red Mask: Size %u, Shift %u\n", mode->red_mask_size, mode->red_mask_shift);
    //     printf("  Green Mask: Size %u, Shift %u\n", mode->green_mask_size, mode->green_mask_shift);
    //     printf("  Blue Mask: Size %u, Shift %u\n", mode->blue_mask_size, mode->blue_mask_shift);
    // }

}

void print_firmware_type(){
    if(firmware_type_request.response != NULL){
        print("Firmwire Type: ");
        uint64_t firmware_type = firmware_type_request.response->firmware_type;
        if(firmware_type == LIMINE_FIRMWARE_TYPE_X86BIOS){
            print("X86BIOS\n");
        }else if(firmware_type == LIMINE_FIRMWARE_TYPE_UEFI32){
            print("UEFI32\n");
        }else if(firmware_type == LIMINE_FIRMWARE_TYPE_UEFI64){
            print("UEFI64\n");
        } 
    }else{
        print("No firmware type found!\n");
    }
}

void print_memory_map() {
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
            case 0x1:
                print(" (Available)\n");
                break;
            case 0x2:
                print(" (Reserved)\n");
                break;
            case 0x3:
                print(" (ACPI Reclaimable)\n");
                break;
            case 0x4:
                print(" (ACPI NVS)\n");
                break;
            case 0x5:
                print(" (Bad Memory)\n");
                break;
            case 0x1000:
                print("Bootloader reclaimable\n");
                break;
            case 0x1001:
                print("Kernel/Modules\n");
                break;
            case 0x1002:
                print("Framebuffer\n");
                break;
            default:
                print(" (Unknown Type !!)\n");
        }
    }
}


void print_kernel_address(){
    if (kernel_address_request.response != NULL) {
        uint64_t physical_base = kernel_address_request.response->physical_base;
        uint64_t virtual_base = kernel_address_request.response->virtual_base;

        // Calculate the offset between virtual and physical addresses.
        uint64_t kernel_offset = virtual_base - physical_base;

        print("Kernel Offset : ");
        print_hex(kernel_offset);

        uint64_t* test_address = (uint64_t*) 0xFFFFFFFF80000000;  // Higher-half virtual address
        *test_address = 45;

        print("\nVirtual Address : ");
        print_hex((uint64_t) &test_address);
        print("\nPhysical Address :");
        uint64_t test_phy_addr = 0xFFFFFFFF80000000 - kernel_offset;
        print_hex(test_phy_addr);
        uint64_t test_data = *test_address;
        print("\ntest data : ");
        print_dec(test_data);
        print("\n");

    }else{
        print("No Kernel address memory map not found!\n");
    }
}


void print_cpu_info() {
    if(smp_request.response != NULL){
        uint64_t revision = smp_request.response->revision;
        uint32_t flags = smp_request.response->flags;
        uint32_t bsp_lapic_id = smp_request.response->bsp_lapic_id; // The Local APIC ID of the bootstrap processor.
        uint64_t cpu_count = smp_request.response->cpu_count; //  How many CPUs are present. It includes the bootstrap processor.
        struct limine_smp_info **cpus = smp_request.response->cpus; // Pointer to an array of cpu_count pointers to struct limine_smp_info structures.

        print("\nCPU Count: ");
        print_dec(cpu_count);
        print("\n");

        for(size_t i=0;i<cpu_count;i++){
            uint32_t processor_id = cpus[0]->processor_id;
            uint32_t lapic_id = cpus[0]->lapic_id;
            uint64_t reserved = cpus[0]->reserved;
            limine_goto_address goto_address = cpus[0]->goto_address;
            uint64_t extra_argument = cpus[0]->extra_argument;

            print("Processor ID : ");
            print_dec(processor_id);
            print(" Lapic ID: ");
            print_dec( lapic_id);
            print("\n");
        }
    }else{
        print("No smp request response found!\n");
    }
}


void print_paging_mode(){
    if(paging_mode_request.response != NULL){
        uint64_t mode = paging_mode_request.response->mode;
        if(mode == LIMINE_PAGING_MODE_X86_64_4LVL){
            print("Limine Paging mode x86_64 4 Level\n");
        } else if(mode == LIMINE_PAGING_MODE_X86_64_5LVL){
            print("Limine Paging mode x86_64 5 Level\n");
        }
    }else{
        print("No Paging mode found!\n");
    }
}


void print_bootloader_info(){
    if(bootloader_info_request.response != NULL){
        uint64_t revision = bootloader_info_request.response->revision;
        char *name = bootloader_info_request.response->name;
        char *version = bootloader_info_request.response->version;
        print("Bootloader name : ");
        print(name);
        print(" Version : ");
        print(version);
        print("\n");
    }else{
        print("No Bootloader Info found!\n");
    }
}


void print_stack_info(){
    if(stack_size_request.response != NULL){
        uint64_t stack_size = stack_size_request.stack_size;
        print("Stack Size : ");
        print_dec(stack_size);
    }else{
        print("No Stack Size Info found.");
    }
}


void print_hhdm(){
    if(hhdm_request.response != NULL){
        uint64_t revision = hhdm_request.response->revision;
        uint64_t offset = hhdm_request.response->offset;
        print("\nRevision : ");
        print_dec(revision);
        print("\nOffset : ");
        print_hex(offset);
    }else{
        print("No HHDM Request found!\n");
    }
}


