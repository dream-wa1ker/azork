#include "cli.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "error.h"


static const char *require_argument(int argc, char **argv, int *index, const char *option) {
    if (*index + 1 >= argc) {
        fprintf(stderr,
                "azork: option '%s' requires "
                "an argument\n",
                option);

        exit(EXIT_FAILURE);
    }

    (*index)++;

    return argv[*index];
}


static void add_recipient(Options *options, const char *recipient){
    if (options->recipient_count ==
        options->recipient_capacity) {

        size_t new_capacity;

        if (options->recipient_capacity == 0)
            new_capacity = 4;
        else
            new_capacity =
                options->recipient_capacity * 2;


        const char **new_list =
            realloc(
                options->recipients,
                new_capacity *
                    sizeof(*new_list)
            );

        if (!new_list)
            die("out of memory");

        options->recipients =
            new_list;

        options->recipient_capacity =
            new_capacity;
    }


    options->recipients[
        options->recipient_count++
    ] = recipient;
}


Options parse_arguments(int argc, char **argv){
    Options options;

    memset(
        &options,
        0,
        sizeof(options)
    );

    options.sample_rate =
        DEFAULT_SAMPLE_RATE;


    for (int i = 1;
         i < argc;
         i++) {

        const char *arg =
            argv[i];


        /*
         * ----------------------------------------------------
         * Encode
         * Does the comparision of the flags, parses the arguments,
         * for the encoding operations...
         * IMPORTANT:
         *
         * -e consumes its argument.
         *
         * Example:
         *
         *     -e ./azork.c
         * ----------------------------------------------------
         */

        if (strcmp(arg, "-e") == 0 ||
            strcmp(arg, "--encode") == 0) {

            if (options.decode)
                die("cannot use --encode and "
                    "--decode together");

            if (options.encode)
                die("--encode specified more than once");

            options.encode = 1;

            options.payload =
                require_argument(
                    argc,
                    argv,
                    &i,
                    arg
                );

            continue;
        }


        /*
         * ----------------------------------------------------
         * Decode
         *
         * IMPORTANT:
         * Similarly, like encode, this is the decode flags comparision.
         * -d consumes its argument.
         *
         * Example:
         *
         *     -d stegudio.wav
         * ----------------------------------------------------
         */

        if (strcmp(arg, "-d") == 0 ||
            strcmp(arg, "--decode") == 0) {

            if (options.encode)
                die("cannot use --encode and "
                    "--decode together");

            if (options.decode)
                die("--decode specified more than once");

            options.decode = 1;

            options.from =
                require_argument(
                    argc,
                    argv,
                    &i,
                    arg
                );

            continue;
        }


        /*
         * ----------------------------------------------------
         * WAV input
         *
         * -f consumes its argument.
         * ----------------------------------------------------
         */

        if (strcmp(arg, "-f") == 0 ||
            strcmp(arg, "--from") == 0) {

            options.from =
                require_argument(
                    argc,
                    argv,
                    &i,
                    arg
                );

            continue;
        }


        /*
         * ----------------------------------------------------
         * Output
         *
         * -t consumes its argument.
         * ----------------------------------------------------
         */

        if (strcmp(arg, "-t") == 0 ||
            strcmp(arg, "--to") == 0) {

            options.to =
                require_argument(
                    argc,
                    argv,
                    &i,
                    arg
                );

            continue;
        }


        /*
         * ----------------------------------------------------
         * GPG
         * ----------------------------------------------------
         */

        if (strcmp(arg, "--gpg") == 0) {
            options.gpg = 1;
            continue;
        }


        /*
         * ----------------------------------------------------
         * Recipient
         *
         * Repeatable.
         * ----------------------------------------------------
         */

        if (strcmp(arg, "--recipient") == 0) {

            const char *recipient =
                require_argument(
                    argc,
                    argv,
                    &i,
                    arg
                );

            add_recipient(
                &options,
                recipient
            );

            continue;
        }


        /*
         * ----------------------------------------------------
         * Signer
         * ----------------------------------------------------
         */

        if (strcmp(arg, "--signer") == 0) {

            options.signer =
                require_argument(
                    argc,
                    argv,
                    &i,
                    arg
                );

            continue;
        }


        /*
         * ----------------------------------------------------
         * Sample rate
         * ----------------------------------------------------
         */

        if (strcmp(arg, "--sample-rate") == 0) {

            const char *value =
                require_argument(
                    argc,
                    argv,
                    &i,
                    arg
                );

            char *end = NULL;

            errno = 0;

            unsigned long rate =
                strtoul(
                    value,
                    &end,
                    10
                );

            if (errno != 0 ||
                end == value ||
                *end != '\0' ||
                rate == 0 ||
                rate > UINT32_MAX) {

                die("invalid --sample-rate");
            }

            options.sample_rate =
                (uint32_t)rate;

            continue;
        }


        /*
         * ----------------------------------------------------
         * Help
         * ----------------------------------------------------
         */

        if (strcmp(arg, "-h") == 0 ||
            strcmp(arg, "--help") == 0) {

            usage(argv[0]);

            exit(EXIT_SUCCESS);
        }


        /*
         * ----------------------------------------------------
         * Unknown argument
         * ----------------------------------------------------
         */

        fprintf(stderr,
                "azork: unknown option: %s\n",
                arg);

        fprintf(stderr,
                "azork: use --help for usage\n");

        exit(EXIT_FAILURE);
    }


    /*
     * --------------------------------------------------------
     * General validation
     * --------------------------------------------------------
     */

    if (!options.encode &&
        !options.decode) {

        die("specify either "
            "--encode/-e or "
            "--decode/-d");
    }


    /*
     * Encode requirements.
     */
    if (options.encode) {

        if (!options.payload)
            die("encode requires "
                "-e/--encode <payload>");

        if (!options.from)
            die("encode requires "
                "-f/--from <audio.wav>");

        if (!options.to)
            die("encode requires "
                "-t/--to <stegudio.wav>");
    }


    /*
     * Decode requirements.
     */
    if (options.decode) {

        if (!options.from)
            die("decode requires "
                "-d/--decode <stegudio.wav>");

        if (!options.to)
            die("decode requires "
                "-t/--to <payload>");
    }


    /*
     * --------------------------------------------------------
     * GPG validation
     * --------------------------------------------------------
     */

    if (!options.gpg) {

        if (options.recipient_count > 0)
            die("--recipient requires --gpg");

        if (options.signer)
            die("--signer requires --gpg");
    }


    /*
     * Recipients and signer only make sense for encoding.
     */
    if (!options.encode) {

        if (options.recipient_count > 0)
            die("--recipient can only be used "
                "while encoding");

        if (options.signer)
            die("--signer can only be used "
                "while encoding");
    }


    /*
     * GPG encoding requires at least one recipient.
     */
    if (options.encode &&
        options.gpg &&
        options.recipient_count == 0) {

        die("--gpg encode requires at least "
            "one --recipient");
    }


    return options;
}
