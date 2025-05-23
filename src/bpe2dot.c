#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "bpe.h"

#include "stb_ds.h"

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

void usage(const char *program_name)
{
    fprintf(stderr, "Usage %s <input.bpe> <output.dot>\n", program_name);
}

int main(int argc, char **argv)
{
    const char* program_name = argv[0];

    if (argc <= 1) {
        usage(program_name);
        fprintf(stderr, "ERROR: no input is provided\n");
        return 1;
    }
    const char* input_file_path = argv[1];

    if (argc == 2) {
        usage(program_name);
        fprintf(stderr, "ERROR: no output is provided\n");
        return 1;
    }
    const char* output_file_path = argv[2];

    Pair *pairs = NULL;
    if(!load_pairs(input_file_path, &pairs)) return 1;

    char *dot_buffer = NULL;
    render_dot(pairs, &dot_buffer);
    if(!write_entire_file(output_file_path, dot_buffer, arrlen(dot_buffer))) return 1;
    printf("INFO: generated %s\n", output_file_path);
    arrfree(dot_buffer);
    return 0;
}
