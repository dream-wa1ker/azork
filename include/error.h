#pragma once
// Print a message prefixed with "azork: " to stderr and exit(EXIT_FAILURE).
void die(const char *message);

// Same as die(), but appends strerror(errno).
void die_errno(const char *message);

// Print full CLI usage/help text to stdout.
void usage(const char *program);


