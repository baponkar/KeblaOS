#pragma once

#include <stdint.h>

#define OS_NAME "KeblaOS"
#define OS_VERSION "0.13"
#define BUILD_DATE "26/01/2025"
#define LAST_UPDATE "09/02/2025"

uint32_t get_lapic_id();
void route_keyboard_irq_to_bsp() ;
void kmain();
