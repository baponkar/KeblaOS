
#include "../limine/limine.h"
#include "../bootloader/memory.h"

#include "kmalloc.h"
#include  "../driver/vga.h"

#include "../lib/assert.h"
#include "../lib/string.h"

#include "../util/util.h"

#include "pmm.h"


// As limine put kernel into higher half so we set userspace at first usable space and set kernel space at second usable space
// This file will set or free a 4KB physical Frame.
// one row of bitmap can store information(free/use) of 8 * 4 KB = 32 Kb memory page(8 pages)
// A bitset of frames - used or free.


uint64_t *frames; // start of bitset frames
uint64_t nframes; // Total frames

uint64_t bitmap_mem_size;


// set the value of frames array by using bit no
void set_frame(uint64_t bit_no) {
    uint64_t bitmap_idx = INDEX_FROM_BIT_NO(bit_no);
    uint64_t bitmap_off = OFFSET_FROM_BIT_NO(bit_no);

    assert(bitmap_off < BITMAP_SIZE); // check either bit_off is less than total BITMAP_SIZE i.e. 64

    frames[bitmap_idx] |= (0x1ULL << bitmap_off); // Set the bit
}

// Static function to clear a bit in the frames bitset
void clear_frame(uint64_t bit_no)
{
   uint64_t bitmap_idx = INDEX_FROM_BIT_NO(bit_no);
   uint64_t bitmap_off = OFFSET_FROM_BIT_NO(bit_no);
   frames[bitmap_idx] &= ~(0x1ULL << bitmap_off);  // clears bit of frames
}

// Static function to test if a bit is set or not.
uint64_t test_frame(uint64_t bit_no)
{
   uint64_t bitmap_idx = INDEX_FROM_BIT_NO(bit_no);
   uint64_t bitmap_off = OFFSET_FROM_BIT_NO(bit_no);
   return (frames[bitmap_idx] & (0x1ULL << bitmap_off));  // returns 0 or 1
}


// Static function to find the first free frame.
// The below function will return a valid bit number or invalid bit no -1
uint64_t free_frame_bit_no()
{
    for (uint64_t bitmap_idx = 0; bitmap_idx < INDEX_FROM_BIT_NO(nframes); bitmap_idx++)
    {
        if (frames[bitmap_idx] != 0xFFFFFFFFFFFFFFFF) // if all bits not set, i.e. there has at least one bit is clear
        {    
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



void init_pmm(){

    print("Strat of PMM initialization...\n");

    uint64_t tmp_i = KMEM_LOW_BASE;

    nframes = (uint64_t) KMEM_LENGTH / FRAME_SIZE;
    frames = (uint64_t*) kmalloc_a( (nframes + 63) * sizeof(uint64_t) / 64 , 1); // Allocate enough bytes for the bitmap
    memset(frames, 0, (nframes + 63) * sizeof(uint64_t) / 64 ); // Zero out the bitmap array

    uint64_t tmp_f = KMEM_LOW_BASE; // KMEM_LOW_BASE changed from initial value of KMEM_LOW_BASE 

    bitmap_mem_size = tmp_f - tmp_i;

    print("Successfully initialized PMM!\n");
}

void test_pmm(){

    print("\nTest Physical Memory Manager(pmm):\n");

    print("Frames Pointer Address : ");
    print_hex((uint64_t) frames);
    print("\n");

    print("Total Frames : ");
    print_dec(nframes);
    print("\n");

    
    print("After frames allocation next free address pointer: ");
    print_hex(KMEM_LOW_BASE);
    print("\n");

    print("Total Memory used for bitmap : ");
    print_hex(bitmap_mem_size);
    print(" [");
    print_size_with_units(bitmap_mem_size);
    print("]");
    print("\n");

}




