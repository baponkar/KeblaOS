
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

8192 - 15 entries
6144 - 14 entries
4096 - 13 entries
2048 - 12 entries
1024 - 12 entries
512 - 12 entries
256 - 9 entries
128 - 8 entries
64 - 7 entries
32 - 6 entries
*/

#include "pmm.h"

// Getting information from boot.c
extern struct limine_memmap_request memmap_request;
extern struct limine_hhdm_request hhdm_request;
extern struct limine_kernel_address_request kernel_address_request;


// As limine put kernel into higher half so we set userspace at first usable space and set kernel space at second usable space

// The kernel will using the below address to store the kernel heap
volatile uint64_t  kernel_placement_address = 0;
uint64_t kernel_end_address;
uint64_t kernel_length;

// The kernel will using the below address to store the user heap
volatile uint64_t user_placement_address;
uint64_t user_end_address;
uint64_t user_length;


// one row of bitmap can store information(free/use) of 8 * 4 KB = 32 Kb memory page(8 pages)
// A bitset of frames - used or free.

uint64_t *frames; // start of bitset frames
uint64_t nframes; // Total frames



void set_frame(uint64_t bit_no) {
    print("Inside set_frame bit_no : ");
    print_dec(bit_no);
    print("\n");
    uint64_t bit_idx = INDEX_FROM_BIT_NO(bit_no);
    uint64_t bit_off = OFFSET_FROM_BIT_NO(bit_no);

    print("bit_idx: ");
    print_dec(bit_idx);
    print(" bit_off: ");
    print_dec(bit_off);
    print("\n");

    assert(bit_off < BITMAP_SIZE);

    frames[bit_idx] |= (0x1ULL << bit_off); // Set the bit

    print("frames[");
    print_dec(bit_idx);
    print("] = ");
    print_bin(frames[bit_idx]);
    print("\n");
}

// Static function to clear a bit in the frames bitset
void clear_frame(uint64_t frame_idx)
{
   uint64_t bitmap_idx = INDEX_FROM_BIT_NO(frame_idx);
   uint64_t bitmap_off = OFFSET_FROM_BIT_NO(frame_idx);
   frames[bitmap_idx] &= ~(0x1ULL << bitmap_off);  // clears bit of frames
}

// Static function to test if a bit is set or not.
uint64_t test_frame(uint64_t frame_idx)
{
   uint64_t bitmap_idx = INDEX_FROM_BIT_NO(frame_idx);
   uint64_t bitmap_off = OFFSET_FROM_BIT_NO(frame_idx);
   return (frames[bitmap_idx] & (0x1ULL << bitmap_off));  // returns 0 or 1
}


// Static function to find the first free frame.
// The below function will return a physical address or invalid frame address -1
uint64_t free_frame_bit_no()
{
    for (uint64_t bitmap_idx = 0; bitmap_idx < INDEX_FROM_BIT_NO(nframes); bitmap_idx++)
    {
        if (frames[bitmap_idx] != 0xFFFFFFFFFFFFFFFF) // if all bits not set
        {    
            // at least one bit is free here.
            for (uint64_t bitmap_off = 0; bitmap_off < BITMAP_SIZE; bitmap_off++)
            {
                uint64_t toTest = (uint64_t) 0x1ULL << bitmap_off; // Ensure the shift is handled as a 64-bit value.ULL means Unsigned Long Long 

                if ( !(frames[bitmap_idx] & toTest) ) // if corresponding bit is zero
                {
                    return CONVERT_BIT_NO(bitmap_idx, bitmap_off); // return corresponding bit number i.e frame index
                    break;
                }
            }
        }
   }
   return (uint64_t)-1; // Return an invalid frame index to indicate failure.
}


void init_mem(){
    size_t entry_ids[4]; // This array will store the index of the first 4 usable memory regions
    // initialise the entry_ids array with 0
    for (size_t i = 0; i < 4; i++)
    {
        entry_ids[i] = 0;
    }
    
    // Check if the memory map response is available
    if (memmap_request.response == NULL) {
        print("Memory map request failed.\n");
        return;
    }

    uint64_t entry_count = memmap_request.response->entry_count;
    struct limine_memmap_entry **entries = memmap_request.response->entries;

    size_t tmp = 0;

    for (size_t i = 0; i < entry_count; i++)
    {
        struct limine_memmap_entry *entry = entries[i];

        if(entry->type == LIMINE_MEMMAP_USABLE){
            entry_ids[tmp] = i; // store the index of the first 4 usable memory regions
            tmp++;
        }
    }

    if(entry_ids[3] == 0){
        entry_ids[3] = entry_ids[2];
    }

    // place kernel into higher  usable memory space
    kernel_placement_address = entries[entry_ids[3]]->base;
    kernel_length = entries[entry_ids[3]]->length;
    kernel_end_address = kernel_placement_address + kernel_length;

    // place user into lower half usable memory space
    user_placement_address = entries[entry_ids[1]]->base;
    user_length = entries[entry_ids[1]]->length;
    user_end_address = user_placement_address + user_length;

    nframes = (uint64_t) kernel_length / FRAME_SIZE;
    frames = (uint64_t*) kmalloc_a(nframes * BITMAP_SIZE, 1); // Allocate enough bytes for the bitmap
    memset(frames, 0, nframes * BITMAP_SIZE); // Zero out the bitmap
    
    print("Successfully initialized memory!\n");
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

    uint64_t entry_count = memmap_request.response->entry_count;
    struct limine_memmap_entry **entries = memmap_request.response->entries;

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



uint64_t HIGHER_HALF_DIRECT_MAP_REVISION;
uint64_t HIGHER_HALF_DIRECT_MAP_OFFSET;

void get_hhdm_info(void){
    if(hhdm_request.response == NULL){
        print("Higher Half Direct Map request failed.\n");
        return;
    }

    HIGHER_HALF_DIRECT_MAP_REVISION = hhdm_request.response->revision;
    HIGHER_HALF_DIRECT_MAP_OFFSET = hhdm_request.response->offset;
}



uint64_t VIRTUAL_BASE;
uint64_t PHYSICAL_BASE;
uint64_t VIRTUAL_TO_PHYSICAL_OFFSET = 0;

uint64_t get_vir_to_phy_offset(){
     if (kernel_address_request.response == NULL) {
        print("Kernel address request failed.\n");
        return 0;
    }
    
    PHYSICAL_BASE = kernel_address_request.response->physical_base;
    VIRTUAL_BASE = kernel_address_request.response->virtual_base;

    // Calculate the offset between virtual and physical addresses.
    VIRTUAL_TO_PHYSICAL_OFFSET = VIRTUAL_BASE - PHYSICAL_BASE;

    return VIRTUAL_TO_PHYSICAL_OFFSET;
}


