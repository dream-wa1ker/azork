#pragma once

#include <stdint.h>

uint16_t read_u16_le(const uint8_t *p);
uint32_t read_u32_le(const uint8_t *p);

void write_u16_le(uint8_t *p, uint16_t value);
void write_u32_le(uint8_t *p, uint32_t value);


