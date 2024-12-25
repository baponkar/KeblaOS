
/*
I have seen that the bootloader is providing the memory map and higher half 
direct map offset information to the kernel. So, I have implemented the code to 
get the memory map and higher half direct map offset information from the bootloader. 
I have also implemented the code to get the virtual to physical offset information from 
the bootloader. I have also implemented the code to print the memory map, higher half direct 
map offset, and virtual to physical offset information. I have also implemented the code to set 
the frame, clear the frame, test the frame, and find the first free frame. I have also implemented 
the code to print the size with units. I have also implemented the code to print the virtual to physical 
offset information. I have also implemented the code to get the virtual to physical offset information from 
the bootloader. I have also implemented the code to get the higher half direct map offset information from 
the bootloader. I have also implemented the code to print the memory map information.

The memory map is giving 
(0x1000)   4KB - 328 KB : Bootloader reclaimable, can be usable.
(0x52000)  328 KB -  636 KB : Usable.
(0x9FC00)  639 KB -  640 KB : Reserved, not be modified!
(0XF0000)  960 KB -  1024 KB / 1 MB : Reserved, not be modified!

(0X100000) 1 MB - 2.9872 GB : Usable. This memory will be use for userspace programs.

(0XBF28000)  2.9872 GB - 2.9903 GB(3.1744 MB) : Kernel/Modules, not usable!
(0XBF619000) 2.9903 GB - 2.9934 GB(3.1744 MB) : Bootloader reclaimable, can be usable.
(0XBF946000) 2.9934 GB - 2.9992 GB(5.9392 MB) : Usable.
(0XBFF31000) 2.9992 GB - 2.9999 GB(0.7168 MB) : Bootloader reclaimable, can be usable.
(0XBFFE0000) 2.9999 GB - 3.9531 GB(976.0768 MB) : Reserved, not be modified!
(0XFD000000) 3.9531 GB - 3.9998 GB(47.8208 MB) : Bootloader reclaimable, can be usable.
(0XFFFC0000) 3.9998 GB - 4.0000 GB(0.2048 MB) : Bootloader reclaimable, can be usable.
(0X10000000) 4.0000 GB - 5.0000 GB(1 GB) : Bootloader reclaimable, can be usable.
*/

#include "pmm.h"

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(0);

__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 3
};

__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 3
};


// The kernel will using the below address to store the kernel heap
uint64_t placement_address = 0X10000000;  // 4 GB   // The value of it will set by bootloader memory map
uint64_t mem_end_address = 0X140000000; // 3 GB    // The value of it will set by bootloader memory map


// one row of bitmap can store information(free/use) of 8 * 4 KB = 32 Kb memory page(8 pages)
// A bitset of frames - used or free.

uint64_t *frames; // start of bitset frames
uint64_t nframes; // Total frames


// Static function to set a bit in the frames bitset
void set_frame(uint64_t frame_addr)
{
   uint64_t frame = frame_addr/0x1000;
   uint64_t idx = INDEX_FROM_BIT(frame);
   uint64_t off = OFFSET_FROM_BIT(frame);
   frames[idx] |= (0x1 << off);    // set bit of frames
}

// Static function to clear a bit in the frames bitset
void clear_frame(uint64_t frame_addr)
{
   uint64_t frame = frame_addr/0x1000;
   uint64_t idx = INDEX_FROM_BIT(frame);
   uint64_t off = OFFSET_FROM_BIT(frame);
   frames[idx] &= ~(0x1 << off);  // clears bit of frames
}

// Static function to test if a bit is set.
uint64_t test_frame(uint64_t frame_addr)
{
   uint64_t frame = frame_addr/0x1000;
   uint64_t idx = INDEX_FROM_BIT(frame);
   uint64_t off = OFFSET_FROM_BIT(frame);
   return (frames[idx] & (0x1 << off));  // returns 0 or 1
}

// Static function to find the first free frame.
// The below function will return a physical address or invalid frame address -1
uint64_t first_frame()
{
   uint64_t i, j;
   for (i = 0; i < INDEX_FROM_BIT(nframes); i++)
   {
       if (frames[i] != 0xFFFFFFFFFFFFFFFF) // if all bits set or not
       {
           // at least one bit is free here.
           for (j = 0; j < 64; j++)
           {
               uint64_t toTest = 0x1ULL << j; // Ensure the shift is handled as a 64-bit value.ULL means Unsigned Long Long 
               if ( !(frames[i] & toTest) ) // if corresponding bit is zero
               {
                   return i * 64 + j; // return corresponding address from index i and offset j
               }
           }
       }
   }
   return (uint64_t)-1; // Return an invalid frame index to indicate failure.
}



void print_size_with_units(uint64_t size) {
    const char *units[] = {"Bytes", "KB", "MB", "GB", "TB"};
    int unit_index = 0;


    // Determine the appropriate unit
    while (size >= 1024 && unit_index < 4) {
        size /= 1024;
        unit_index++;
    }

    // Print the size with the unit
    print_dec((uint64_t)size); // Print the integer part
    print(" ");
    print(units[unit_index]);
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





uint64_t HIGHER_HALF_DIRECT_MAP_REVISION;
uint64_t HIGHER_HALF_DIRECT_MAP_OFFSET;

void get_hhdm_info(void){
    if(hhdm_request.response != NULL){
        HIGHER_HALF_DIRECT_MAP_REVISION = hhdm_request.response->revision;
        HIGHER_HALF_DIRECT_MAP_OFFSET = hhdm_request.response->offset;
    }else{
        HIGHER_HALF_DIRECT_MAP_REVISION = 0;
        HIGHER_HALF_DIRECT_MAP_OFFSET = 0;
    }
}

__attribute__((used, section(".requests")))
static volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 3
};

uint64_t VIRTUAL_BASE;
uint64_t PHYSICAL_BASE;
uint64_t VIRTUAL_TO_PHYSICAL_OFFSET;

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

void print_virtual_to_physical_offset(void){
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
