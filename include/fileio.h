#pragma once

#include <stddef.h>
#include <stdint.h>

#include "types.h"

// Reads an entire file into a heap-allocated buffer.
// Exits the process on any I/O error (via die/die_errno).
Buffer read_entire_file(const char *path);

// Writes `size` bytes from `data` to `path`, creating/truncating it.
// Exits the process on any I/O error (via die/die_errno).
void write_entire_file(const char *path, const uint8_t *data, size_t size);


