#include "pmm.h"

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



