#pragma once

#include "../stdlib/stdint.h"
#include "ports.h"

#define SA_UP_FREQ 240
#define RE_FREQ 270
#define GA_FREQ 300
#define MA_FREQ 320
#define PA_FREQ 360
#define DHA_FREQ 400
#define NI_FREQ 450
#define SA_DOWN_FREQ 480

void pc_speaker_enable_sound(uint32_t freq);
void pc_speaker_disable_sound();