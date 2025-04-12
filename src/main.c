#include <stdio.h>
#include <string.h>
#include <stdint.h>

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

int main(void)
{
    const char *text = "The original BPE algorithm operates by iteratively replacing the most common contiguous sequences of characters in a target text with unused 'placeholder' bytes. The iteration ends when no sequences can be found, leaving the target text effectively compressed. Decompression can be performed by reversing this process, querying known placeholder terms against their corresponding denoted sequence, using a lookup table. In the original paper, this lookup table is encoded and stored alongside the compressed text.";
    int text_size = strlen(text);

    Freq *freq = NULL;
    Freq *freqs_sorted = NULL;

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

    // Put pairs and occurence in freqs_sorted array
    for (ptrdiff_t i = 0; i < hmlen(freq); ++i) {
        arrput(freqs_sorted, freq[i]);
    }

    // Find index of pair with max occurence in hashmap
    ptrdiff_t max_index = 0;
    for (ptrdiff_t i = 1; i < hmlen(freq); ++i) {
        if (freq[i].value > freq[max_index].value) {
            max_index = i;
        }
    }

    printf("(%u, %u) => %zu\n", freq[max_index].key.l, freq[max_index].key.r, freq[max_index].value);

    // Put pair with max occurence in pairs array at new index (first time its index 256)
    arrput(pairs, freq[max_index].key);

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

    render_tokens(pairs, tokens_in);
    render_tokens(pairs, tokens_out);

    printf("Uncompressed text size: %d\n", arrlen(tokens_in));
    printf("Compressed text size: %d\n", arrlen(tokens_out));

    // qsort(freqs_sorted, arrlen(freqs_sorted), sizeof(Freq), compare_freqs);
    //
    // for (size_t i = 0; i < arrlen(freqs_sorted); ++i) {
    //     Freq *freqs = &freqs_sorted[i];
    //     printf("(%u, %u) => %zu\n", freqs->key.l, freqs->key.r, freqs->value);
    // }
    return 0;
}
