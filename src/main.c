#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>

#include "cli.h"
#include "codec.h"
#include "error.h"
#include "types.h"


int main(int argc, char **argv)
{
    if (argc == 1) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }


    Options options =
        parse_arguments(
            argc,
            argv
        );


    if (options.encode)
        encode(&options);
    else
        decode(&options);


    free(options.recipients);

    return EXIT_SUCCESS;
}
