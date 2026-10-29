#include "error.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void die(const char *message)
{
    fprintf(stderr, "azork: %s\n", message);
    exit(EXIT_FAILURE);
}


void die_errno(const char *message)
{
    fprintf(stderr,
            "azork: %s: %s\n",
            message,
            strerror(errno));

    exit(EXIT_FAILURE);
}


void usage(const char *program)
{
    printf(
        "AZORK - acoustic steganography\n"
        "\n"
        "Usage:\n"
        "\n"
        "  Encode:\n"
        "    %s -e <payload> -f <audio.wav> -t <stegudio.wav>\n"
        "\n"
        "  Decode:\n"
        "    %s -d <stegudio.wav> -t <payload>\n"
        "\n"
        "  GPG encode:\n"
        "    %s -e <payload> -f <audio.wav> -t <stegudio.wav> \\\n"
        "--gpg --recipient <key>\n"
        "\n"
        "  GPG decode:\n"
        "    %s -d <stegudio.wav> -t <payload> --gpg\n"
        "\n"
        "Options:\n"
        "\n"
        "  -e, --encode <file>       Payload to encode\n"
        "  -d, --decode <file>       WAV file to decode\n"
        "  -f, --from <file>         WAV input when encoding\n"
        "  -t, --to <file>           Output file\n"
        "\n"
        "      --gpg                 Use GPG\n"
        "      --recipient <key>     GPG recipient; repeatable\n"
        "      --signer <key>        GPG signing identity\n"
        "\n"
        "      --sample-rate <Hz>    Expected sample rate\n"
        "                            (default: 96000)\n"
        "\n"
        "  -h, --help                Show this help\n"
        "\n"
        "Examples:\n"
        "\n"
        "  %s -e secret.txt -f music.wav -t stegudio.wav\n"
        "\n"
        "  %s -d stegudio.wav -t recovered.txt\n"
        "\n"
        "  %s -e secret.txt -f music.wav -t stegudio.wav \\\n"
        "--gpg --recipient alice@example.com\n"
        "\n"
        "  %s -d stegudio.wav -t recovered.txt --gpg\n"
        "\n",
        program,
        program,
        program,
        program,
        program,
        program,
        program,
        program
    );
}
