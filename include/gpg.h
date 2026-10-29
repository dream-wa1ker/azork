#pragma once

#include <stddef.h>
#include <stdint.h>

#include "types.h"

// Forks/execs `gpg` with the given argv (NULL-terminated) and waits
// for it to finish. Returns the child's exit status (or 128+signal
// if it was killed by a signal).
int run_gpg(char *const argv[]);

// Runs `gpg --sign --encrypt` on the file at `payload_path` for the
// recipients/signer configured in `options`, and returns the
// resulting OpenPGP binary data as a heap-allocated Buffer.
// Exits the process if no recipients are configured or if GPG fails.
Buffer gpg_encrypt_and_sign(const char *payload_path, const Options *options);

// Runs `gpg --decrypt` (which also verifies any embedded signature)
// on `data_size` bytes of OpenPGP data, writing the recovered
// plaintext to `output_path`.
// Exits the process if GPG fails.
void gpg_decrypt_and_verify(const uint8_t *data, size_t data_size, const char *output_path);


