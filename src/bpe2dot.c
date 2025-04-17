#include <stdio.h>
#include <bpe.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

int read_entire_file(const char *file_path, void **data, size_t *data_size)
{
    int fd = 0;
    struct stat statbuf = {0};

    fd = open(file_path, O_RDONLY, S_IRUSR | S_IRGRP);
    if (fd == -1)
    {
        printf("failed to open %s\n", file_path);
        exit(EXIT_FAILURE);
    }

    if (fstat(fd, &statbuf) == -1)
    {
        printf("failed to fstat %s\n", file_path);
        exit(EXIT_FAILURE);
    }

    *data_size = statbuf.st_size;
    if (close(fd) == -1)
    {
        printf("failed to fclose %s\n", file_path);
        exit(EXIT_FAILURE);
    }

    FILE* fp = fopen(file_path, "rb");
    *data = malloc(*data_size);
    size_t read_bytes = fread(*data, 1, *data_size, fp);
    printf("read_bytes: %zu\n", read_bytes);

    if (fp) fclose(fp);
}

bool load_pairs(const char *file_path, Pair **pairs, char **buffer, size_t buf_size)
{
    if(read_entire_file(file_path,(void**)buffer, &buf_size) != 0) return false;
    if(buf_size*sizeof(char)%sizeof(Pair) != 0) {
        fprintf(stderr, "ERROR: size of %s (%zu) must be divisible by %zu\n", file_path, buf_size*sizeof(char), sizeof(Pair));
        return false;
    }
    arrsetlen(*pairs,buf_size*sizeof(char)/sizeof(Pair));
    memcpy(*pairs, buffer, buf_size*sizeof(char));
    return true;
}

void generate_dot(Pair *pairs)
{
    printf("digraph Pairs {\n");
    for (uint32_t token = 0; token < arrlen(pairs); ++token) {
        if (!(token == pairs[token].l)) {
            printf("  %u -> %u\n", token, pairs[token].l);
            printf("  %u -> %u\n", token, pairs[token].r);
        }
    }
    printf("}\n");
}

int main(int argc, char **argv)
{
    const char* program_name = argv[0];

    if (argc <= 0) {
        fprintf(stderr, "Usage: %s <input.bin> <output.dot>\n", program_name);
        fprintf(stderr, "ERROR: no input is provided\n");
    }
    const char* input_file_path = argv[1];

    if (argc == 2) {
        fprintf(stderr, "Usage: %s <input.bin> <output.dot>\n", program_name);
        fprintf(stderr, "ERROR: no output is provided\n");
    }
    const char* output_file_path = argv[2];

    char *out_buffer = NULL;
    size_t out_buffer_size = 0;

   if(!load_pairs("../pairs.bin", &pairs, &out_buffer, out_buffer_size)) return 1;


    return 0;
}
