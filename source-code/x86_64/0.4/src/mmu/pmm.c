#include "pmm.h"

// one row of bitmap can store information(free/use) of 8 * 4 KB = 32 Kb memory page(8 pages)


// A bitset of frames - used or free.
extern uint64_t *frames;
extern uint64_t nframes;


// Static function to set a bit in the frames bitset
void set_frame(uint64_t frame_addr)
{
   uint64_t frame = frame_addr/0x1000;
   uint64_t idx = INDEX_FROM_BIT(frame);
   uint64_t off = OFFSET_FROM_BIT(frame);
   frames[idx] |= (0x1 << off);
}

// Static function to clear a bit in the frames bitset
void clear_frame(uint64_t frame_addr)
{
   uint64_t frame = frame_addr/0x1000;
   uint64_t idx = INDEX_FROM_BIT(frame);
   uint64_t off = OFFSET_FROM_BIT(frame);
   frames[idx] &= ~(0x1 << off);
}

// Static function to test if a bit is set.
uint64_t test_frame(uint64_t frame_addr)
{
   uint64_t frame = frame_addr/0x1000;
   uint64_t idx = INDEX_FROM_BIT(frame);
   uint64_t off = OFFSET_FROM_BIT(frame);
   return (frames[idx] & (0x1 << off));
}

// Static function to find the first free frame.
uint64_t first_frame()
{
   uint64_t i, j;
   for (i = 0; i < INDEX_FROM_BIT(nframes); i++)
   {
       if (frames[i] != 0xFFFFFFFFFFFFFFFF) // nothing free, exit early.
       {
           // at least one bit is free here.
           for (j = 0; j < 64; j++)
           {
               uint64_t toTest = 0x1 << j;
               if ( !(frames[i] & toTest) )
               {
                   return i*8*8+j;
               }
           }
       }
   }
   return 0;
}



