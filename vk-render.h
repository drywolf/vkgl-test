#pragma once

#include <stdint.h>

bool vk_init(uint32_t w, uint32_t h, uint32_t num_samples);
void vk_shutdown();
