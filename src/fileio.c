#include "fileio.h"

#include <stdio.h>
#include <stdlib.h>

#include "error.h"


Buffer read_entire_file(const char *path)
{
    FILE *fp = fopen(path, "rb");

    if (!fp)
        die_errno(path);

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        die_errno("fseek");
    }

    long file_size = ftell(fp);

    if (file_size < 0) {
        fclose(fp);
        die_errno("ftell");
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        die_errno("fseek");
    }

    size_t size = (size_t)file_size;

    uint8_t *data = malloc(size ? size : 1);

    if (!data) {
        fclose(fp);
        die("out of memory");
    }

    if (size > 0 && fread(data, 1, size, fp) != size) {
        free(data);
        fclose(fp);
        die_errno("reading file");
    }

    fclose(fp);

    Buffer result = {
        .data = data,
        .size = size
    };

    return result;
}


void write_entire_file(const char *path,
                        const uint8_t *data,
                        size_t size)
{
    FILE *fp = fopen(path, "wb");

    if (!fp)
        die_errno(path);

    if (size > 0 && fwrite(data, 1, size, fp) != size) {
        fclose(fp);
        die_errno("writing file");
    }

    if (fclose(fp) != 0)
        die_errno("closing output file");
}
