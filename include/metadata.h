#pragma once
#include <stddef.h>
#include <stdint.h>

#include "common.h"

// Fills a 100-byte AZORK metadata header (magic, version, flags,
// payload type; remaining bytes reserved/zeroed).
void create_metadata(uint8_t metadata[AZORK_METADATA_SIZE], uint8_t payload_type);

// Validates an extracted metadata header's magic/version/payload type.
// Exits the process on any invalid/unsupported field.
//
// Returns the payload type (AZORK_PAYLOAD_RAW or AZORK_PAYLOAD_OPENPGP).
uint8_t validate_metadata(const uint8_t metadata[AZORK_METADATA_SIZE]);

// Given the size of a WAV's PCM data chunk (in bytes), returns how many
// payload bytes can be hidden after reserving space for the AZORK
// metadata header and the 4-byte payload size field. Returns 0 if the
// WAV is too small to hold even the header.
size_t calculate_capacity_bytes(uint32_t data_size);


