#include "codec.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "endian.h"
#include "error.h"
#include "fileio.h"
#include "gpg.h"
#include "metadata.h"
#include "stego.h"
#include "wav.h"


void encode(const Options *options)
{
    if (!options->payload)
        die("encode requires -e/--encode <payload>");

    if (!options->from)
        die("encode requires -f/--from <audio.wav>");

    if (!options->to)
        die("encode requires -t/--to <stegudio.wav>");


    /*
     * --------------------------------------------------------
     * Read WAV
     * --------------------------------------------------------
     */

    fprintf(stderr,
            "azork: reading WAV: %s\n",
            options->from);

    Buffer wav =
        read_entire_file(options->from);

    WavInfo info =
        parse_wav(
            &wav,
            options->sample_rate
        );


    /*
     * --------------------------------------------------------
     * Capacity
     * --------------------------------------------------------
     */

    size_t capacity =
        calculate_capacity_bytes(
            info.data_size
        );

    if (capacity == 0) {
        free(wav.data);

        die("WAV does not have enough capacity "
            "for AZORK metadata");
    }


    fprintf(stderr,
            "azork: sample rate: %" PRIu32 " Hz\n",
            info.sample_rate);

    fprintf(stderr,
            "azork: audio data: %" PRIu32 " bytes\n",
            info.data_size);

    fprintf(stderr,
            "azork: payload capacity: %zu bytes\n",
            capacity);


    /*
     * --------------------------------------------------------
     * Prepare payload
     * --------------------------------------------------------
     */

    Buffer payload;

    uint8_t payload_type;


    if (options->gpg) {
        /*
         * GPG receives the PAYLOAD file.
         *
         * It does NOT receive the WAV.
         */
        fprintf(stderr,
                "azork: GPG mode enabled\n");

        payload =
            gpg_encrypt_and_sign(
                options->payload,
                options
            );

        payload_type =
            AZORK_PAYLOAD_OPENPGP;
    }
    else {
        payload =
            read_entire_file(
                options->payload
            );

        payload_type =
            AZORK_PAYLOAD_RAW;
    }


    fprintf(stderr,
            "azork: hidden payload: %zu bytes\n",
            payload.size);


    /*
     * Payload size is stored as uint32_t.
     */
    if (payload.size > UINT32_MAX) {
        free(payload.data);
        free(wav.data);

        die("payload is too large for AZORK's "
            "32-bit payload size field");
    }


    /*
     * --------------------------------------------------------
     * Check capacity
     * --------------------------------------------------------
     */

    if (payload.size > capacity) {
        fprintf(stderr,
                "azork: payload is too large\n"
                "azork: capacity: %zu bytes\n"
                "azork: required: %zu bytes\n",
                capacity,
                payload.size);

        free(payload.data);
        free(wav.data);

        exit(EXIT_FAILURE);
    }


    /*
     * --------------------------------------------------------
     * Calculate sample positions
     * --------------------------------------------------------
     */

    const size_t metadata_samples =
        AZORK_METADATA_SIZE * 8;

    const size_t size_samples =
        4 * 8;

    const size_t payload_start =
        metadata_samples +
        size_samples;

    const size_t required_samples =
        payload_start +
        payload.size * 8;

    const size_t available_samples =
        info.data_size / 4;


    if (required_samples >
        available_samples) {

        free(payload.data);
        free(wav.data);

        die("internal capacity calculation error");
    }


    /*
     * --------------------------------------------------------
     * Metadata
     * --------------------------------------------------------
     */

    uint8_t metadata[
        AZORK_METADATA_SIZE
    ];

    create_metadata(
        metadata,
        payload_type
    );


    /*
     * Pointer to actual PCM data.
     *
     * This is important:
     *
     * WAV metadata/chunks before the data chunk remain
     * completely untouched.
     */
    uint8_t *audio =
        wav.data + info.data_offset;


    /*
     * --------------------------------------------------------
     * Embed metadata
     *
     * samples 0..799
     * --------------------------------------------------------
     */

    embed_bytes(
        audio,
        0,
        metadata,
        AZORK_METADATA_SIZE
    );


    /*
     * --------------------------------------------------------
     * Embed payload size
     *
     * samples 800..831
     * --------------------------------------------------------
     */

    uint8_t size_bytes[4];

    write_u32_le(
        size_bytes,
        (uint32_t)payload.size
    );

    embed_bytes(
        audio,
        metadata_samples,
        size_bytes,
        sizeof(size_bytes)
    );


    /*
     * --------------------------------------------------------
     * Embed payload
     *
     * samples 832...
     * --------------------------------------------------------
     */

    embed_bytes(
        audio,
        payload_start,
        payload.data,
        payload.size
    );


    /*
     * --------------------------------------------------------
     * Write WAV
     * --------------------------------------------------------
     */

    write_entire_file(
        options->to,
        wav.data,
        wav.size
    );


    fprintf(stderr,
            "azork: wrote %s\n",
            options->to);

    fprintf(stderr,
            "azork: payload type: %s\n",
            payload_type == AZORK_PAYLOAD_OPENPGP
                ? "OpenPGP"
                : "raw");

    fprintf(stderr,
            "azork: samples used: %zu / %zu\n",
            required_samples,
            available_samples);


    free(payload.data);
    free(wav.data);
}


void decode(const Options *options)
{
    if (!options->from)
        die("decode requires -d/--decode <stegudio.wav>");

    if (!options->to)
        die("decode requires -t/--to <payload>");


    /*
     * --------------------------------------------------------
     * Read WAV
     * --------------------------------------------------------
     */

    fprintf(stderr,
            "azork: reading WAV: %s\n",
            options->from);

    Buffer wav =
        read_entire_file(options->from);

    WavInfo info =
        parse_wav(
            &wav,
            options->sample_rate
        );


    size_t available_samples =
        info.data_size / 4;

    const size_t metadata_samples =
        AZORK_METADATA_SIZE * 8;

    const size_t size_samples =
        4 * 8;


    /*
     * Need enough samples for:
     *
     *   metadata + payload size
     */
    if (available_samples <
        metadata_samples + size_samples) {

        free(wav.data);

        die("WAV does not contain enough data "
            "for AZORK header");
    }


    uint8_t *audio =
        wav.data + info.data_offset;


    /*
     * --------------------------------------------------------
     * Extract metadata
     * --------------------------------------------------------
     */

    uint8_t metadata[
        AZORK_METADATA_SIZE
    ];

    extract_bytes(
        audio,
        0,
        metadata,
        AZORK_METADATA_SIZE
    );


    uint8_t payload_type =
        validate_metadata(metadata);


    fprintf(stderr,
            "azork: AZORK metadata valid\n");

    fprintf(stderr,
            "azork: payload type: %s\n",
            payload_type == AZORK_PAYLOAD_OPENPGP
                ? "OpenPGP"
                : "raw");


    /*
     * --------------------------------------------------------
     * Extract payload size
     * --------------------------------------------------------
     */

    uint8_t size_bytes[4];

    extract_bytes(
        audio,
        metadata_samples,
        size_bytes,
        sizeof(size_bytes)
    );


    uint32_t payload_size =
        read_u32_le(size_bytes);


    fprintf(stderr,
            "azork: hidden payload size: "
            "%" PRIu32 " bytes\n",
            payload_size);


    /*
     * --------------------------------------------------------
     * Calculate payload location
     * --------------------------------------------------------
     */

    const size_t payload_start =
        metadata_samples +
        size_samples;

    const size_t payload_samples =
        (size_t)payload_size * 8;


    /*
     * Check for overflow and bounds.
     */
    if (payload_samples >
        available_samples - payload_start) {

        free(wav.data);

        die("hidden payload extends beyond "
            "WAV data");
    }


    /*
     * --------------------------------------------------------
     * Extract payload
     * --------------------------------------------------------
     */

    Buffer payload;

    payload.size =
        payload_size;

    payload.data =
        malloc(
            payload.size
                ? payload.size
                : 1
        );

    if (!payload.data) {
        free(wav.data);
        die("out of memory");
    }


    extract_bytes(
        audio,
        payload_start,
        payload.data,
        payload.size
    );


    free(wav.data);


    /*
     * --------------------------------------------------------
     * Raw payload
     * --------------------------------------------------------
     */

    if (payload_type ==
        AZORK_PAYLOAD_RAW) {

        /*
         * Don't accidentally pass raw data to GPG.
         */
        if (options->gpg) {
            fprintf(stderr,
                    "azork: --gpg was requested, "
                    "but hidden payload is raw data\n");

            free(payload.data);

            exit(EXIT_FAILURE);
        }


        write_entire_file(
            options->to,
            payload.data,
            payload.size
        );


        fprintf(stderr,
                "azork: payload written to %s\n",
                options->to);


        free(payload.data);

        return;
    }


    /*
     * --------------------------------------------------------
     * OpenPGP payload
     * --------------------------------------------------------
     */

    if (payload_type ==
        AZORK_PAYLOAD_OPENPGP) {

        if (!options->gpg) {
            fprintf(stderr,
                    "azork: hidden payload is "
                    "OpenPGP data\n"
                    "azork: decode it with --gpg\n");

            free(payload.data);

            exit(EXIT_FAILURE);
        }


        /*
         * GPG decrypts and verifies.
         */
        gpg_decrypt_and_verify(
            payload.data,
            payload.size,
            options->to
        );


        fprintf(stderr,
                "azork: decrypted and verified "
                "payload written to %s\n",
                options->to);


        free(payload.data);

        return;
    }


    /*
     * Should be unreachable because metadata validation
     * already checks the payload type.
     */
    free(payload.data);

    die("unknown AZORK payload type");
}
