#pragma once

#include <stdint.h>

#include "../../../util/util.h"


void pic_irq_remap();
void disable_pic();
void enable_pic();
void send_eoi(uint8_t irq);

