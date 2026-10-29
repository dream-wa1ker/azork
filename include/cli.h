#pragma once
#include "types.h"

// Parses argv into an Options struct, applying all validation rules
// (mutually exclusive flags, required arguments, GPG constraints).
// Exits the process (via die, or directly after printing usage) on
// any invalid combination, or on -h/--help.
Options parse_arguments(int argc, char **argv);

// cli :)
