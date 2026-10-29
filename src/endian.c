#include "endian.h"


uint16_t read_u16_le(const uint8_t *p)
{
    return (uint16_t)p[0]
         | ((uint16_t)p[1] << 8);
}


uint32_t read_u32_le(const uint8_t *p)
{
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}


void write_u16_le(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value & 0xFF);
    p[1] = (uint8_t)((value >> 8) & 0xFF);
}


void write_u32_le(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)(value & 0xFF);
    p[1] = (uint8_t)((value >> 8) & 0xFF);
    p[2] = (uint8_t)((value >> 16) & 0xFF);
    p[3] = (uint8_t)((value >> 24) & 0xFF);
}
