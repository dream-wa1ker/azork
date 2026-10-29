#pragma once

#include <stddef.h>
#include <stdint.h>

// Embed `data_size` bytes into `audio` starting at `sample_offset`,
// one bit per 32-bit sample (stored in the LSB of the sample's first
// byte), MSB first within each byte.
void embed_bytes(uint8_t *audio, size_t sample_offset, const uint8_t *data, size_t data_size);

// does the opposite function of the embed bytes;
void extract_bytes(const uint8_t *audio, size_t sample_offset, uint8_t *data, size_t data_size);


