// header : for defining the data types
#pragma once 

#include <stddef.h> 
#include <stdint.h> 

// fat pointer type Buffer
typedef struct {
    uint8_t *data;
    size_t size;
} Buffer;

// a parsed summary of a WAV file. where the fmt and data chunks are.
// just the audio properties are read from them. 
// NOTE: this is not the same as on disk wav file header. Never overlay them as raw bytes. Acts as a reading container

typedef struct {
    size_t fmt_offset;
    uint32_t fmt_size; 

    size_t data_offset;  // custom
    uint32_t data_size;  // custom 

    uint16_t audio_format;
    uint16_t channels;
    uint32_t sample_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;

    int found_fmt; // custom
    int found_data; // custom 

} WavInfo;
// this is custom wav header format, data from original header -> read into this.

// options struct, for the CLI options
typedef struct {
    int encode;
    int decode; 
    int gpg;

    // payload file encoding
    // let's have it as -e or --encode 
    const char *payload;

    // wav input for encoding or wav input for decoding
    // let's have it as -d or --decode 
    const char *from;

    // output file
    // le'ts have this as -t or --to <somefile.wav>
    const char *to;

    // for the GPG recipients in case gpg flag exists 
    // as there are multiple recipient allowed, we have an pointer to pointer of recipients.
    const char **recipients;
    // this is a dynamic string array (heap allocated based on count and capacity)
    size_t recipient_count;
    size_t recipient_capacity; 

    // GPG signing identity
    const char *signer;

    uint32_t sample_rate;
} Options;

// EOF -- azork/types.h -- 
