#pragma once

#include <stdint.h>
#include <stdbool.h>

void synth_init(void);
void synth_tick(void);
void synth_set_keys(uint32_t mask);
