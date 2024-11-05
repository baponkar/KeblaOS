#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../util/util.h"
#include "../../limine/limine.h"    // https://github.com/limine-bootloader/limine/blob/v8.x/PROTOCOL.md#kernel-address-feature

#include "../driver/vga/vga.h"
#include "../driver/keyboard.h"

#include "../usr/shell.h"

#include "../mmu/cpu.h"
#include "../mmu/paging.h"

#include "syscall.h"

#include "../gdt/gdt.h"
#include "../idt/idt.h"
#include "../idt/timer.h"

#include "info.h"

extern uint64_t ticks;
extern uint64_t seconds;

uint32_t *fb_ptr = NULL;
uint64_t kernel_offset;
uint64_t kernel_virtual_base;
uint64_t kernel_physical_base;

size_t SCREEN_WIDTH;
size_t SCREEN_HEIGHT;

size_t MIN_LINE_NO;
size_t MAX_LINE_NO;

size_t MIN_COLUMN_NO;
size_t MAX_COLUMN_NO;

// Set the base revision to 2, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// Define the kernel address request
__attribute__((used, section(".requests")))
static volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

// Define memory map 
__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.


// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}


void kmain(void) {

    get_framebuffer_info();
    cls();

    extern uint32_t KeblaOS_icon_320x200x32[];
    //display_image( (SCREEN_WIDTH/2 - KEBLAOS_ICON_320X200X32_WIDTH/2), 5, KeblaOS_icon_320x200x32, KEBLAOS_ICON_320X200X32_WIDTH, KEBLAOS_ICON_320X200X32_HEIGHT);

    if (kernel_address_request.response != NULL) {
        kernel_physical_base = kernel_address_request.response->physical_base;
        kernel_virtual_base = kernel_address_request.response->virtual_base;

        // Calculate the offset between virtual and physical addresses.
        kernel_offset = kernel_virtual_base - kernel_physical_base;

        print("Kernel Offset : ");
        print_hex(kernel_offset);
        print("\n");

        uint64_t* test_address = (uint64_t*)0xFFFFFFFF80000000;  // Higher-half virtual address
        *test_address = 0x12345678ABCDEF00;

        print("Virtual Address : ");
        print_hex((uint64_t)&test_address);
        print("\n");
        print("Physical Address :");
        uint64_t test_phy_addr = 0xFFFFFFFF80000000 - kernel_offset;
        print_hex(test_phy_addr);
        print("\n");
        uint64_t test_data = *test_address;
        print("test data : ");
        print_hex(test_data);
        print("\n");

        print_memory_map();

        // uint64_t* invalid_address = (uint64_t*)0xFFFFFFFF90000000;  // Unmapped address
        // *invalid_address = 0x0;  // This should trigger a page fault
    }

    init_gdt();
    // init_paging();
    init_idt();
    init_timer();
    initKeyboard();

    // test_paging();

    syscall_check();

    // test_interrupt();
    // asm volatile ("int $0xF");

    // is_cpuid_present();
   
    // char vendor[13];
    // char brand[49];
    // uint32_t family, model, features;

    // get_cpu_vendor(vendor);
    // printf("CPU Vendor: %s\n", vendor);

    // get_cpu_brand(brand);
    // printf("CPU Brand: %s\n", brand);

    // get_cpu_features(&family, &model, &features);
    // printf("CPU Family: %u\n", family);
    // printf("CPU Model: %u\n", model);
    // printf("CPU Features (EDX): 0x%08X\n", features);

    // // Check for specific features (e.g., SSE, SSE2, etc.)
    // if (features & (1 << 25)) {
    //     printf("SSE supported\n");
    // }
    // if (features & (1 << 26)) {
    //     printf("SSE2 supported\n");
    // }
    // if (features & (1 << 28)) {
    //     printf("HTT (Hyper-Threading) supported\n");
    // }


    hcf();
}


void get_framebuffer_info(){
    // Ensure the framebuffer is initialized
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer
    struct limine_framebuffer* framebuffer = framebuffer_request.response->framebuffers[0];
    fb_ptr = (uint32_t* ) framebuffer->address;

    SCREEN_WIDTH = framebuffer->width;   // 1024
    SCREEN_HEIGHT = framebuffer->height; // 768

    MIN_LINE_NO = 0;
    MAX_LINE_NO = (SCREEN_HEIGHT / (DEFAULT_FONT_HEIGHT + DEFAULT_TEXT_LINE_GAP)) - 1;

    MIN_COLUMN_NO = 0;
    MAX_COLUMN_NO = (SCREEN_WIDTH / DEFAULT_FONT_WIDTH) - 1;
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
        print("Region ");
        print_hex(i);
        print(": Base = ");
        print_hex(entry->base);
        print(", Length = ");
        print_hex(entry->length);

        // Check the type and print it
        switch (entry->type) {
            case 1:
                print(" (Available)\n");
                break;
            case 2:
                print(" (Reserved)\n");
                break;
            case 3:
                print(" (ACPI Reclaimable)\n");
                break;
            case 4:
                print(" (ACPI NVS)\n");
                break;
            case 5:
                print(" (Bad Memory)\n");
                break;
            default:
                print(" (Unknown Type)\n");
        }
    }
}

