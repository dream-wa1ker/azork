#pragma once 

#include <cstdint>
#include <stdint.h>
#include "types.h"

// helper function for RIFF/WAV validation 
// only works with stereo audio, s32-le.
// requires a sample rate matching the expected_sample_rate.
// and this should exit the process on any failure.

WavInfo parse_wav(const Buffer *wav, uint32_t expected_sample_rate);


