#pragma once


// This creates a unique temproary file, via mkdtemp with the given path prefix, 
// returns its heap allocated path, then exits the process on failure;
char *make_temp_path(const char *prefix);


// Unlinks and frees a temp path created by make_temp_path(), then
// sets *path to NULL. Safe to call with a NULL pointer or *path.
void remove_temp_file(char **path);


