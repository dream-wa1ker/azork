#include "tempfile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "error.h"


char *make_temp_path(const char *prefix)
{
    size_t length =
        strlen(prefix) + 7;

    char *path =
        malloc(length);

    if (!path)
        die("out of memory");

    /*
     * mkstemp requires six X characters.
     */
    snprintf(path,
             length,
             "%sXXXXXX",
             prefix);

    int fd =
        mkstemp(path);

    if (fd < 0) {
        free(path);
        die_errno("mkstemp");
    }

    close(fd);

    return path;
}


void remove_temp_file(char **path)
{
    if (!path || !*path)
        return;

    unlink(*path);

    free(*path);

    *path = NULL;
}
