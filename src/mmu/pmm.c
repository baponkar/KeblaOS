
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


// As limine put kernel into higher half so we set userspace at first usable space and set kernel space at second usable space

// one row of bitmap can store information(free/use) of 8 * 4 KB = 32 Kb memory page(8 pages)
// A bitset of frames - used or free.

uint64_t *frames; // start of bitset frames
uint64_t nframes; // Total frames



void set_frame(uint64_t bit_no) {
    uint64_t bit_idx = INDEX_FROM_BIT_NO(bit_no);
    uint64_t bit_off = OFFSET_FROM_BIT_NO(bit_no);

    assert(bit_off < BITMAP_SIZE);

    frames[bit_idx] |= (0x1ULL << bit_off); // Set the bit
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
    nframes = (uint64_t) KERNEL_MEM_LENGTH / FRAME_SIZE;
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













