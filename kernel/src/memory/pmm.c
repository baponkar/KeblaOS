
#include "../../../limine-9.2.3/limine.h"

#include "../memory/detect_memory.h"

#include "kmalloc.h"
#include  "../lib/stdio.h"

#include "../lib/assert.h"
#include "../lib/string.h"

#include "../util/util.h"

#include "pmm.h"


// This file will set or free a 4KB physical Frame.
// one row of bitmap can store information(free/use) of 8 * 4 KB = 32 Kb memory page(8 pages)
// A bitset of frames - used or free.

uint64_t *frames; // start of bitset frames
uint64_t nframes; // Total numbers of frames

extern volatile uint64_t phys_mem_head; // head of physical memory

// set the value of frames array by using bit no
void set_frame(uint64_t bit_no) {

    assert(bit_no < nframes); // check either bit_no is less than total nframes i.e. 0 to nframes-1

    uint64_t bitmap_idx = INDEX_FROM_BIT_NO(bit_no);
    uint64_t bitmap_off = OFFSET_FROM_BIT_NO(bit_no);

    frames[bitmap_idx] |= (0x1 << bitmap_off);       // Set the bit
}



// Static function to clear a bit in the frames bitset
void clear_frame(uint64_t bit_no)
{
    assert(bit_no < nframes); // check either bit_no is less than total nframes i.e. 0 to nframes-1

    uint64_t bitmap_idx = INDEX_FROM_BIT_NO(bit_no);
    uint64_t bitmap_off = OFFSET_FROM_BIT_NO(bit_no);

    if(frames == NULL){
        printf("frames is null\n");
        return;
    }

    frames[bitmap_idx] &= (0 << bitmap_off);         // clears bit of frames
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
    uint64_t free_bit = (uint64_t)-1;
    bool found = false;

    for (uint64_t bitmap_idx = 0; (bitmap_idx < INDEX_FROM_BIT_NO(nframes)) && !found; bitmap_idx++)
    {
        if (frames[bitmap_idx] != 0xFFFFFFFFFFFFFFFF) // if all bits not set, i.e. there has at least one bit is clear
        {    
            for (uint64_t bitmap_off = 0; bitmap_off < BITMAP_SIZE; bitmap_off++)
            {
                uint64_t toTest = (uint64_t) 0x1ULL << bitmap_off; // Ensure the shift is handled as a 64-bit value.ULL means Unsigned Long Long 

                if ( !(frames[bitmap_idx] & toTest) ) // if corresponding bit is zero
                {
                    free_bit = CONVERT_BIT_NO(bitmap_idx, bitmap_off); // return corresponding bit number i.e frame index
                    found = true;
                    break;
                }
                continue; // If the current bit is set, continue to the next bit in the bitmap.
            }
        }
        continue;   // If all bits in the current bitmap are set, continue to the next bitmap index.
   }
   return free_bit; // Return an invalid frame index to indicate failure.
}



void init_pmm(){

    nframes = (uint64_t) (USABLE_LENGTH_PHYS_MEM) / FRAME_SIZE;         // Total number of frames in the memory

    frames = (uint64_t*) kmalloc_a(sizeof(uint64_t) * (nframes + 1) / BITMAP_SIZE, 1); // Allocate memory for the bitmap array
    if(frames == NULL){
        printf("[Error] PMM: Failed to allocate memory for frames\n");
        return;
    }
    // clear the memory of frames array
    memset(frames, 0, sizeof(uint64_t) * (nframes + 1) / BITMAP_SIZE);                           

    printf(" [-] Successfully initialized PMM!\n");
}


void test_pmm(){
    printf(" Test Physical Memory Manager(pmm):\n");
    printf(" Frames Pointer Address : %x\n", (uint64_t) frames);
    printf(" Total Frames : %d\n", nframes);
    printf(" After frames allocation next free address pointer: %x\n", phys_mem_head);
}




