#include <stdio.h>
#include "bpe.h"

void usage(const char *program_name)
{
    fprintf(stderr, "Usage %s <input.bpe> <output.dot>\n", program_name);
}

int main(int argc, char **argv)
{
    const char* program_name = argv[0];

    if (argc <= 1) {
        fprintf(stderr, "Usage: %s <input.bin> <output.dot>\n", program_name);
        fprintf(stderr, "ERROR: no input is provided\n");
    }
    const char* input_file_path = argv[1];

    return 0;
}
