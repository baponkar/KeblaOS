#include "memory_manager.h"



void invalidate(uint32_t vaddr){
    asm volatile("invlpg" "%0" :: "m"(vaddr));
}