#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "bpe.h"

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
