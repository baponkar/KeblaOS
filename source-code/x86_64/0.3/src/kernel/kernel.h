#pragma once





extern size_t SCREEN_WIDTH;
extern size_t SCREEN_HEIGHT;

extern size_t MIN_LINE_NO;
extern size_t MAX_LINE_NO;

extern size_t MIN_COLUMN_NO;
extern size_t MAX_COLUMN_NO;

extern uint32_t *fb_ptr;
extern uint64_t kernel_physical_base;
extern uint64_t kernel_virtual_base;
extern uint64_t kernel_offset;

void get_framebuffer_info();
void print_memory_map();
