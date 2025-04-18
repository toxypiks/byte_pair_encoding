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

bool load_pairs(const char *file_path, Pair **pairs)
{
    char* buffer = NULL;
    size_t buf_size = 0;
    if(read_entire_file(file_path,(void**)&buffer, &buf_size) != 0) return false;
    if(buf_size*sizeof(char)%sizeof(Pair) != 0) {
        fprintf(stderr, "ERROR: size of %s (%zu) must be divisible by %zu\n", file_path, buf_size*sizeof(char), sizeof(Pair));
        return false;
    }
    arrsetlen(*pairs, buf_size*sizeof(char)/sizeof(Pair));
    memcpy(*pairs, buffer, buf_size*sizeof(char));
    free(buffer);
    return true;
}

void render_dot(const Pair *pairs, char **buffer)
{
    char temp[10000];
    int size = sprintf(temp, "digraph Pairs {\n");
    arrsetlen(*buffer, size);
    memcpy(*buffer, temp, size);
    for (uint32_t token = 0; token < arrlen(pairs); ++token) {
        if (!(token == pairs[token].l)) {
            int size_l = sprintf(temp, "  %u -> %u\n", token, pairs[token].l);
            arrsetlen(*buffer, size + size_l);
            memcpy(*buffer+size, temp, size_l);
            size += size_l;
            int size_r = sprintf(temp, "  %u -> %u\n", token, pairs[token].r);
            arrsetlen(*buffer, size + size_r);
            memcpy(*buffer+size, temp, size_r);
            size += size_r;
        }
    }
    int size_end = sprintf(temp, "}\n");
    arrsetlen(*buffer, size + size_end);
    memcpy(*buffer+size, temp, size_end);
}

bool write_entire_file(const char *file_path, const void *data, size_t size)
{
    bool result = true;

    FILE *fp = fopen(file_path, "wb");
    if (fp == NULL) {
        goto defer;
    }

    const char *buf = data;
    while (size > 0) {
        size_t n = fwrite(buf, 1, size, fp);
        if (ferror(fp)) {
            goto defer;
        }
        size -= n;
        buf  += n;
    }
    result = false;

defer:
    if (fp) fclose(fp);
    return result;

}

int main(int argc, char **argv)
{
    const char* program_name = argv[0];

    if (argc <= 1) {
        fprintf(stderr, "Usage: %s <input.bin> <output.dot>\n", program_name);
        fprintf(stderr, "ERROR: no input is provided\n");
    }
    const char* input_file_path = argv[1];

    if (argc == 2) {
        fprintf(stderr, "Usage: %s <input.bin> <output.dot>\n", program_name);
        fprintf(stderr, "ERROR: no output is provided\n");
    }
    const char* output_file_path = argv[2];

    Pair *pairs = NULL;
    if(!load_pairs("../pairs.bin", &pairs)) return 1;

    char *dot_buffer = NULL;
    render_dot(pairs, &dot_buffer);
    if(!write_entire_file(output_file_path, dot_buffer, arrlen(dot_buffer))) return 1;
    printf("INFO: generated %s\n", output_file_path);
    arrfree(dot_buffer);
    return 0;
}
