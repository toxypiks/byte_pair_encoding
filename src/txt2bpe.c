#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>
#include <assert.h>

#include "bpe.h"
#include "stb_ds.h"

// Hashmap
typedef struct Freq {
    Pair key;
    size_t value;
} Freq;

void render_tokens(Pair *pairs, uint32_t *tokens)
{
    for (size_t i = 0; i < arrlen(tokens); ++i) {
        assert(tokens[i] < arrlen(pairs));
        if (pairs[tokens[i]].l == tokens[i]) {
            printf("%c", tokens[i]);
        } else {
            printf("[%u]", tokens[i]);
        }
    }
    printf("\n");
}

int compare_freqs(const void *a, const void *b)
{
    const Freq *af = a;
    const Freq *bf = b;
    return (int)bf->value - (int)af->value;
}

#define swap(Type, x, y) \
    do { \
        Type t = (x); \
        (x) = (y); \
        (y) = t; \
    } while (0)

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

bool dump_pairs(const char *file_path, Pair* pairs) {
  return write_entire_file(file_path, pairs, arrlen(pairs)*sizeof(Pair));
}

void usage(const char *program_name)
{
    fprintf(stderr, "Usage %s <input.txt> <output.bpe>\n", program_name);
}

void report_progress(size_t iteration, uint32_t *tokens_in, Pair *pairs)
{
    printf("INFO: iteration %zu\n", iteration);
    printf("    Text tokens count: %zu\n", arrlen(tokens_in));
    printf("    BPE table size: %zu\n", arrlen(pairs));
}

int main(int argc, char **argv)
{
    const char *program_name = argv[0];

    if (argc <= 1) {
        usage(program_name);
        fprintf(stderr, "ERROR: no input provided\n");
        return 1;
    }

    const char *input_file_path = argv[1];

    if (argc == 2) {
        usage(program_name);
        fprintf(stderr, "ERROR: no output is privided\n");
        return 1;
    }

    const char *output_file_path = argv[2];

    char *text = NULL;
    size_t text_size = 0;

    Freq *freq = NULL;
    Pair *pairs = NULL;

    uint32_t *tokens_in = NULL;
    uint32_t *tokens_out = NULL;

    if(read_entire_file(input_file_path, (void**)&text, &text_size)) return 1;

    // Initialization of table
    for (uint32_t i = 0; i < 256; ++i) {
        arrput(pairs, ((Pair) {.l = i}));
    }

    // Put text into token_in array
    for (int i = 0; i < text_size; ++i) {
        arrput(tokens_in, text[i]);
    }

    size_t iteration = 0;
    for (;; ++iteration) {
#define REPORT_FREQ 1
        if (iteration%REPORT_FREQ == 0) report_progress(iteration, tokens_in, pairs);

        hmfree(freq);
        // Put two chars of token_in into Pair and put in Hashmap if not already there, if there increment counter for pair in hashmap (value)
        for (size_t i = 0; i < arrlen(tokens_in) - 1; ++i) {
            Pair pair = {
                .l = tokens_in[i],
                .r = tokens_in[i+1]
            };
            ptrdiff_t i = hmgeti(freq, pair);
            if (i < 0) hmput(freq, pair, 1);
            else freq[i].value += 1;
        }

        // Find index of pair with max occurence in hashmap
        ptrdiff_t max_index = 0;
        for (ptrdiff_t i = 1; i < hmlen(freq); ++i) {
            if (freq[i].value > freq[max_index].value) {
                max_index = i;
            }
        }

        if (freq[max_index].value <= 1) break; // No further compression can be done

        // Put pair with max occurence in pairs array at new index (first time its index 256)
        arrput(pairs, freq[max_index].key);

        // clean up tokens_out for next compressing iteration
        arrsetlen(tokens_out, 0);

        // find pair with max occurence in tokens_in and replace it with token and put it into tokens_out otherwise just put pairs into tokens_out
        for (size_t i = 0; i < arrlen(tokens_in);) {
            if (i + 1 >= arrlen(tokens_in)) {
                arrput(tokens_out, tokens_in[i]);
                i += 1;
            } else {
                Pair pair = {.l = tokens_in[i], .r = tokens_in[i + 1]};
                if (memcmp(&pair, &freq[max_index].key, sizeof(pair)) == 0) {
                    arrput(tokens_out, arrlen(pairs) - 1);
                    i += 2;
                } else {
                    arrput(tokens_out, tokens_in[i]);
                    i += 1;
                }
            }
        }
        swap(uint32_t*, tokens_in, tokens_out);
    }
    report_progress(iteration, tokens_in, pairs);

    if(dump_pairs(output_file_path, pairs)) return 1;
    printf("INFO: generated %s\n", output_file_path);

   return 0;
}
