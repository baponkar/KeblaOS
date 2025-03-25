#pragma once

#include <stdint.h>

#include "../../util/util.h"


void pic_irq_remap();
void disable_pic();

void init_pic_interrupt();
