#include "wav.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "endian.h"
#include "error.h"


/*
 * WAVE_FORMAT_EXTENSIBLE PCM SubFormat GUID.
 */
const uint8_t PCM_SUBFORMAT_GUID[16] = {
    0x01, 0x00, 0x00, 0x00,
    0x00, 0x00,
    0x10, 0x00,
    0x80, 0x00,
    0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71
};


static int chunk_id_is(const uint8_t *p, const char *id)
{
    return p[0] == (uint8_t)id[0]
        && p[1] == (uint8_t)id[1]
        && p[2] == (uint8_t)id[2]
        && p[3] == (uint8_t)id[3];
}


WavInfo parse_wav(const Buffer *wav,
                   uint32_t expected_sample_rate)
{
    WavInfo info;

    memset(&info, 0, sizeof(info));

    if (wav->size < 12)
        die("file is too small to be a WAV file");

    if (!chunk_id_is(wav->data, "RIFF"))
        die("not a RIFF file");

    if (!chunk_id_is(wav->data + 8, "WAVE"))
        die("RIFF file is not WAVE format");

    size_t pos = 12;

    while (pos + 8 <= wav->size) {
        const uint8_t *chunk =
            wav->data + pos;

        uint32_t chunk_size =
            read_u32_le(chunk + 4);

        size_t chunk_data =
            pos + 8;

        if ((size_t)chunk_size >
            wav->size - chunk_data) {
            die("WAV chunk extends beyond end of file");
        }

        if (chunk_id_is(chunk, "fmt ")) {
            if (!info.found_fmt) {
                info.found_fmt = 1;

                info.fmt_offset = chunk_data;
                info.fmt_size = chunk_size;

                if (chunk_size < 16)
                    die("WAV fmt chunk is too small");

                const uint8_t *fmt =
                    wav->data + chunk_data;

                info.audio_format =
                    read_u16_le(fmt + 0);

                info.channels =
                    read_u16_le(fmt + 2);

                info.sample_rate =
                    read_u32_le(fmt + 4);

                info.block_align =
                    read_u16_le(fmt + 12);

                info.bits_per_sample =
                    read_u16_le(fmt + 14);
            }
        }
        else if (chunk_id_is(chunk, "data")) {
            if (!info.found_data) {
                info.found_data = 1;

                info.data_offset = chunk_data;
                info.data_size = chunk_size;
            }
        }

        /*
         * RIFF chunks are word-aligned.
         */
        size_t advance =
            8 + (size_t)chunk_size;

        if (chunk_size & 1)
            advance++;

        if (advance > wav->size - pos)
            die("invalid WAV chunk layout");

        pos += advance;
    }

    if (!info.found_fmt)
        die("WAV fmt chunk not found");

    if (!info.found_data)
        die("WAV data chunk not found");


    /*
     * --------------------------------------------------------
     * Audio format
     * --------------------------------------------------------
     */

    if (info.audio_format == WAV_FORMAT_PCM) {
        /*
         * Standard PCM.
         */
    }
    else if (info.audio_format == WAV_FORMAT_EXTENSIBLE) {
        /*
         * WAVE_FORMAT_EXTENSIBLE needs at least 40 bytes:
         *
         *   16-byte base fmt
         *   2-byte cbSize
         *   22-byte extension
         */
        if (info.fmt_size < 40)
            die("WAVE_FORMAT_EXTENSIBLE fmt chunk is too small");

        const uint8_t *fmt =
            wav->data + info.fmt_offset;

        uint16_t cb_size =
            read_u16_le(fmt + 16);

        if (cb_size < 22)
            die("invalid WAVE_FORMAT_EXTENSIBLE extension");

        const uint8_t *subformat =
            fmt + 24;

        if (memcmp(subformat,
                   PCM_SUBFORMAT_GUID,
                   sizeof(PCM_SUBFORMAT_GUID)) != 0) {
            die("WAVE_FORMAT_EXTENSIBLE audio is not PCM");
        }
    }
    else {
        fprintf(stderr,
                "azork: unsupported WAV audio format: "
                "0x%04X\n",
                info.audio_format);

        exit(EXIT_FAILURE);
    }


    /*
     * --------------------------------------------------------
     * Audio properties
     * --------------------------------------------------------
     */

    if (info.channels != 2)
        die("WAV must be stereo");

    if (info.bits_per_sample != 32)
        die("WAV must use 32-bit samples");

    if (info.block_align != 8)
        die("WAV must have 8-byte stereo frame alignment");

    if (info.sample_rate != expected_sample_rate) {
        fprintf(stderr,
                "azork: unsupported sample rate: "
                "%" PRIu32 " Hz "
                "(expected %" PRIu32 " Hz)\n",
                info.sample_rate,
                expected_sample_rate);

        exit(EXIT_FAILURE);
    }

    if (info.data_size == 0)
        die("WAV data chunk is empty");

    if (info.data_size % info.block_align != 0)
        die("WAV data is not aligned to complete stereo frames");

    return info;
}
