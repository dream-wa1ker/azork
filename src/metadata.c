#include "metadata.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "endian.h"
#include "error.h"


void create_metadata(
    uint8_t metadata[AZORK_METADATA_SIZE],
    uint8_t payload_type)
{
    memset(metadata,
           0,
           AZORK_METADATA_SIZE);

    /*
     * Bytes 0-3:
     *
     *       magic = AZRK
     */
    memcpy(metadata + 0,
           AZORK_MAGIC,
           4);

    /*
     * Bytes 4-5:
     *
     *       AZORK version
     */
    write_u16_le(
        metadata + 4,
        AZORK_VERSION
    );

    /*
     * Bytes 6-7:
     *
     *       flags
     *
     * Currently unused.
     */
    write_u16_le(
        metadata + 6,
        0
    );

    /*
     * Bytes 8-11:
     *
     *       payload type
     *
     *       0 = raw
     *       1 = OpenPGP
     */
    write_u32_le(
        metadata + 8,
        payload_type
    );

    /*
     * Bytes 12-99 are currently reserved.
     */
}


uint8_t validate_metadata(
    const uint8_t metadata[AZORK_METADATA_SIZE])
{
    if (memcmp(metadata + 0,
               AZORK_MAGIC,
               4) != 0) {
        die("invalid AZORK magic");
    }

    uint16_t version =
        read_u16_le(metadata + 4);

    if (version != AZORK_VERSION) {
        fprintf(stderr,
                "azork: unsupported AZORK metadata "
                "version: %u\n",
                version);

        exit(EXIT_FAILURE);
    }

    uint32_t payload_type =
        read_u32_le(metadata + 8);

    if (payload_type != AZORK_PAYLOAD_RAW &&
        payload_type != AZORK_PAYLOAD_OPENPGP) {

        fprintf(stderr,
                "azork: unsupported AZORK payload "
                "type: %" PRIu32 "\n",
                payload_type);

        exit(EXIT_FAILURE);
    }

    return (uint8_t)payload_type;
}


size_t calculate_capacity_bytes(
    uint32_t data_size)
{
    /*
     * One 32-bit sample = 4 bytes.
     *
     * Therefore:
     *
     *       samples = data_size / 4
     *
     * Reserved:
     *
     *       metadata = 100 * 8 = 800 samples
     *       size     =   4 * 8 =  32 samples
     */
    size_t samples =
        data_size / 4;

    const size_t reserved_samples =
        (AZORK_METADATA_SIZE * 8) + 32;

    if (samples <= reserved_samples)
        return 0;

    samples -= reserved_samples;

    return samples / 8;
}
