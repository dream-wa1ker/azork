#ifndef AZORK_CODEC_H
#define AZORK_CODEC_H

#include "types.h"


// Reads options->from as a WAV, hides options->payload inside it
// (encrypting/signing first via GPG if options->gpg is set), and
// writes the result to options->to.
void encode(const Options *options);

// Reads options->from as a stego WAV, extracts the hidden AZORK
// payload, decrypts/verifies it via GPG if needed, and writes the
// recovered payload to options->to.
void decode(const Options *options);

#endif /* AZORK_CODEC_H */

// the above functions are the code of the azork codec, the main
// function, takes in options, then encodes or decodes, based on
// the options passed to it.
