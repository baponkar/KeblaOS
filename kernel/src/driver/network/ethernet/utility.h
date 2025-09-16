#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../../io/ports.h"


extern void outb(uint16_t port, uint8_t value);
extern void outw(uint16_t port, uint16_t value);
extern void outl(uint16_t port, uint32_t value);
extern void outq(uint16_t port, uint64_t value);

extern uint8_t inb(uint16_t port);
extern uint16_t inw(uint16_t port);
extern uint32_t inl(uint16_t port);
extern uint64_t inq(uint16_t port);


uint8_t read8 (uint64_t p_address);
uint16_t read16 (uint64_t p_address);
uint32_t read32 (uint64_t p_address);
uint64_t read64 (uint64_t p_address);

void write8 (uint64_t p_address, uint8_t p_value);
void write16 (uint64_t p_address, uint16_t p_value);
void write32 (uint64_t p_address, uint32_t p_value);
void write64 (uint64_t p_address, uint64_t p_value);

