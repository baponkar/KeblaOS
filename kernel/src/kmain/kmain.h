#pragma once

#include <stdint.h>
#include <stdatomic.h>
#include <cpuid.h>

#define OS_NAME "KeblaOS"
#define OS_VERSION "0.17.0"
#define BUILD_DATE "26/01/2025"
#define LAST_UPDATE "11/07/2025"


extern uint8_t core_id;
uint32_t get_lapic_id();
void route_keyboard_irq_to_bsp() ;
void kmain();
