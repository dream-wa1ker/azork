#include "stego.h"

/*
 * Each 32-bit sample is four bytes.
 *
 * Since WAV PCM is little-endian:
 *
 *       sample[0] = least significant byte
 *
 * We modify only sample[0]'s least significant bit.
 *
 * Maximum audio disturbance:
 *
 *       one least-significant bit
 *
 * per sample.
 */

static void set_sample_lsb(uint8_t *sample,
                           uint8_t bit)
{
    sample[0] =
        (uint8_t)((sample[0] & 0xFE) |
                  (bit & 1));
}


static uint8_t get_sample_lsb(const uint8_t *sample)
{
    return (uint8_t)(sample[0] & 1);
}


void embed_bytes(uint8_t *audio,
                  size_t sample_offset,
                  const uint8_t *data,
                  size_t data_size)
{
    size_t sample = sample_offset;

    for (size_t i = 0; i < data_size; i++) {
        uint8_t byte = data[i];

        for (int bit = 7; bit >= 0; bit--) {
            uint8_t value =
                (uint8_t)((byte >> bit) & 1);

            set_sample_lsb(
                audio + sample * 4,
                value
            );

            sample++;
        }
    }
}


void extract_bytes(const uint8_t *audio,
                    size_t sample_offset,
                    uint8_t *data,
                    size_t data_size)
{
    size_t sample = sample_offset;

    for (size_t i = 0; i < data_size; i++) {
        uint8_t byte = 0;

        for (int bit = 7; bit >= 0; bit--) {
            uint8_t value =
                get_sample_lsb(
                    audio + sample * 4
                );

            byte |=
                (uint8_t)(value << bit);

            sample++;
        }

        data[i] = byte;
    }
}
