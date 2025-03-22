#pragma once

#include <stdint.h>

#include "../../util/util.h"

void pic_irq_remap();

void isr_handler(registers_t *regs);
void isr_install();

void irq_handler(registers_t *regs);
void irq_remap();

void irq_install();
void init_pic_interrupt();

void disable_pic();


