#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef struct Pair {
    uint32_t l, r;
} Pair;

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

bool write_entire_file(const char *file_path, const void *data, size_t size)
{
    bool result = true;

    FILE *f = fopen(file_path, "wb");
    if (f == NULL) {
        goto defer;
    }

    const char *buf = data;
    while (size > 0) {
        size_t n = fwrite(buf, 1, size, f);
        if (ferror(f)) {
            goto defer;
        }
        size -= n;
        buf  += n;
    }
    result = false;

defer:
    if (f) fclose(f);
    return result;

}

bool read_entire_file(const char *file_path, void *buffer, size_t buffer_size)
{
    bool result = true;

    FILE *f = fopen(file_path, "r");
    if (f == NULL) {
        goto defer;
    }

    char *buf = buffer;
    char *n = fgets(buf, buffer_size, f);
    if (ferror(f)) {
        goto defer;
    }
    result = false;

defer:
    if (f) fclose(f);
    return result;
}

bool dump_pairs(const char *file_path, Pair* pairs) {
  return write_entire_file(file_path, pairs, arrlen(pairs)*sizeof(Pair));
}

bool load_pairs(const char *file_path, Pair *pairs)
{
    return read_entire_file(file_path, pairs, arrlen(pairs)*sizeof(Pair));
}

int main(void)
{
    const char *text = "The original BPE algorithm operates by iteratively replacing the most common contiguous sequences of characters in a target text with unused 'placeholder' bytes. The iteration ends when no sequences can be found, leaving the target text effectively compressed. Decompression can be performed by reversing this process, querying known placeholder terms against their corresponding denoted sequence, using a lookup table. In the original paper, this lookup table is encoded and stored alongside the compressed text.";
    int text_size = strlen(text);

    Freq *freq = NULL;

    uint32_t *tokens_in = NULL;
    uint32_t *tokens_out = NULL;

    Pair *pairs = NULL;

    // Initialization of table
    for (uint32_t i = 0; i < 256; ++i) {
        arrput(pairs, ((Pair) {.l = i}));
    }

    // Put text into token_in array
    for (int i = 0; i < text_size; ++i) {
        arrput(tokens_in, text[i]);
    }

    for (;;) {
        //render_tokens(pairs, tokens_in); // Render tokens throughout each compression iteration

        //printf("%zu\n", arrlen(tokens_in));

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
    generate_dot(pairs);
    if(!dump_pairs("../pairs.bin", pairs)) return 1;

    return 0;
}
