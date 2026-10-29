#include "gpg.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "error.h"
#include "fileio.h"
#include "tempfile.h"


int run_gpg(char *const argv[])
{
    pid_t pid =
        fork();

    if (pid < 0)
        die_errno("fork");

    if (pid == 0) {
        /*
         * Child process.
         *
         * GPG stderr remains connected to AZORK's stderr.
         */
        execvp("gpg", argv);

        fprintf(stderr,
                "azork: failed to execute gpg: %s\n",
                strerror(errno));

        _exit(127);
    }

    int status;

    while (waitpid(pid, &status, 0) < 0) {
        if (errno == EINTR)
            continue;

        die_errno("waitpid");
    }

    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    if (WIFSIGNALED(status)) {
        fprintf(stderr,
                "azork: gpg terminated by signal %d\n",
                WTERMSIG(status));

        return 128 + WTERMSIG(status);
    }

    return 1;
}


Buffer gpg_encrypt_and_sign(
    const char *payload_path,
    const Options *options)
{
    if (options->recipient_count == 0)
        die("--gpg encode requires at least one "
            "--recipient");

    char *gpg_output =
        make_temp_path("/tmp/azork-gpg-");


    /*
     * Argument count:
     *
     *       gpg
     *       --batch
     *       --yes
     *       --output
     *       output
     *       --sign
     *       --encrypt
     *
     *       optional:
     *           --local-user signer
     *
     *       recipients:
     *           --recipient key
     *           ...
     *
     *       payload
     *       NULL
     */
    size_t max_args =
        16 +
        (options->signer ? 2 : 0) +
        (options->recipient_count * 2);

    char **argv =
        calloc(max_args,
               sizeof(char *));

    if (!argv) {
        free(gpg_output);
        die("out of memory");
    }

    size_t i = 0;

    argv[i++] = "gpg";
    argv[i++] = "--batch";
    argv[i++] = "--yes";

    argv[i++] = "--output";
    argv[i++] = gpg_output;

    /*
     * GPG performs:
     *
     *       sign
     *       encrypt
     */
    argv[i++] = "--sign";
    argv[i++] = "--encrypt";

    /*
     * Optional signing identity.
     */
    if (options->signer) {
        argv[i++] = "--local-user";
        argv[i++] = (char *)options->signer;
    }

    /*
     * Recipients.
     */
    for (size_t r = 0;
         r < options->recipient_count;
         r++) {

        argv[i++] = "--recipient";
        argv[i++] =
            (char *)options->recipients[r];
    }

    /*
     * Original payload.
     */
    argv[i++] = (char *)payload_path;

    argv[i] = NULL;


    fprintf(stderr,
            "azork: running GPG sign/encrypt...\n");

    int result =
        run_gpg(argv);

    free(argv);

    if (result != 0) {
        remove_temp_file(&gpg_output);

        fprintf(stderr,
                "azork: GPG sign/encrypt failed "
                "(exit code %d)\n",
                result);

        exit(EXIT_FAILURE);
    }


    Buffer result_buffer =
        read_entire_file(gpg_output);

    remove_temp_file(&gpg_output);


    if (result_buffer.size == 0) {
        free(result_buffer.data);
        die("GPG produced empty output");
    }

    return result_buffer;
}


void gpg_decrypt_and_verify(
    const uint8_t *data,
    size_t data_size,
    const char *output_path)
{
    char *encrypted_path =
        make_temp_path("/tmp/azork-gpg-in-");

    char *decrypted_path =
        make_temp_path("/tmp/azork-gpg-out-");


    /*
     * Write extracted OpenPGP data to temporary file.
     */
    write_entire_file(
        encrypted_path,
        data,
        data_size
    );


    /*
     * GPG --decrypt:
     *
     *   decrypts the message
     *   verifies an embedded signature
     *
     * if the message was signed.
     */
    char *argv[] = {
        "gpg",
        "--batch",
        "--yes",
        "--output",
        decrypted_path,
        "--decrypt",
        encrypted_path,
        NULL
    };


    fprintf(stderr,
            "azork: running GPG decrypt/verify...\n");

    int result =
        run_gpg(argv);


    if (result != 0) {
        remove_temp_file(&encrypted_path);
        remove_temp_file(&decrypted_path);

        fprintf(stderr,
                "azork: GPG decrypt/verify failed "
                "(exit code %d)\n",
                result);

        exit(EXIT_FAILURE);
    }


    /*
     * GPG succeeded.
     *
     * Only now read the resulting plaintext.
     */
    Buffer decrypted =
        read_entire_file(decrypted_path);


    remove_temp_file(&encrypted_path);
    remove_temp_file(&decrypted_path);


    write_entire_file(
        output_path,
        decrypted.data,
        decrypted.size
    );

    free(decrypted.data);
}
