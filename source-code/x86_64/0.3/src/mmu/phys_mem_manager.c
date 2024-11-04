
#include "phys_mem_manager.h"

// one row of bitmap can store information(free/use) of 8 * 4 KB = 32 Kb memory page(8 pages)

uint8_t bitmaps[];

// Converting bit no to physical address
uint64_t bit_no_to_addr(uint64_t bit_no){
    return bit_no * PAGE_SIZE;
}

// converting physical address into uint64_t address
uint64_t addr_to_bit_no(uint64_t addr){
    return addr / PAGE_SIZE;
}

// Converting bitmap_loc_t structure into physical address
uint64_t bitmap_loc_to_addr(bitmap_loc_t bitmap_loc){
    uint64_t bit_no = bitmap_loc.row * BIT_PER_ROW + bitmap_loc.col;
    uint64_t addr = bit_no * PAGE_SIZE;
    return addr;
}

// Converting physical address into  bitmap_loc_t structure
bitmap_loc addr_to_bitmap_loc(uint64_t addr){
    bitmap_loc_t _loc;
    uint64_t bitmap_loc = addr / PAGE_SIZE;
    _loc.row = bitmap_loc / BIT_PER_ROW;
    _loc.col = bitmap_loc % BIT_PER_ROW;  
    return _loc;
}