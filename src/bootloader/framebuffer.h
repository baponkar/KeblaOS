#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

extern size_t FRAMEBUFFER_WIDTH;
extern size_t FRAMEBUFFER_HEIGHT;
extern uint32_t *FRAMEBUFFER_PTR;
extern size_t FRAMEBUFFER_PITCH;
extern size_t FRAMEBUFFER_BPP; // Should be 32 (RGBA)

void get_framebuffer_info();
