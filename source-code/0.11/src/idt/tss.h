#pragma once

#include "../stdlib/stdint.h"
#include "../util/util.h"
#include "../gdt/gdt.h"


void write_tss(int32_t num, uint16_t ss0, uint32_t esp0);
void init_tss();
void switch_to_user_mode();

