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

    uint32_t *tokens = NULL;

    Pair *pairs = NULL;

    // Initialization of table
    for (uint32_t i = 0; i < 256; ++i) {
        arrput(pairs, ((Pair) {.l = i}));
    }

    for (int i = 0; i < text_size; ++i) {
        arrput(tokens, text[i]);
    }

    for (size_t i = 0; i < arrlen(tokens) - 1; ++i) {
        Pair pair = {
            .l = tokens[i],
            .r = tokens[i+1]
        };
        ptrdiff_t i = hmgeti(freq, pair);
        if (i < 0) hmput(freq, pair, 1);
        else freq[i].value += 1;
    }

    for (ptrdiff_t i = 0; i < hmlen(freq); ++i) {
        arrput(freqs_sorted, freq[i]);
    }

    ptrdiff_t max_index = 0;
    for (ptrdiff_t i = 1; i < hmlen(freq); ++i) {
        if (freq[i].value > freq[max_index].value) {
            max_index = i;
        }
    }

    printf("(%u, %u) => %zu\n", freq[max_index].key.l, freq[max_index].key.r, freq[max_index].value);

    // qsort(freqs_sorted, arrlen(freqs_sorted), sizeof(Freq), compare_freqs);
    //
    // for (size_t i = 0; i < arrlen(freqs_sorted); ++i) {
    //     Freq *freqs = &freqs_sorted[i];
    //     printf("(%u, %u) => %zu\n", freqs->key.l, freqs->key.r, freqs->value);
    // }
    return 0;
}
