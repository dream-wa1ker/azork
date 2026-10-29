#pragma once
#include <stdint.h>

// Constants

#define AZORK_METADATA_SIZE 100
#define AZORK_MAGIC "AZRK"
#define AZORK_VERSION 1

#define AZORK_PAYLOAD_RAW      0
#define AZORK_PAYLOAD_OPENPGP  1

#define WAV_FORMAT_PCM         0x0001
#define WAV_FORMAT_EXTENSIBLE  0xFFFE

#define DEFAULT_SAMPLE_RATE 96000

// WAVE_FORMAT_EXTENSIBLE PCM SubFormat GUID.
extern const uint8_t PCM_SUBFORMAT_GUID[16];


