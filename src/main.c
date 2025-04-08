#include <stdio.h>
#include <string.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef struct Pair {
    char pair[2];
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

Freq *freq = NULL;
Freq *freqs_sorted = {0};

int main(void)
{
    const char *text = "The original BPE algorithm operates by iteratively replacing the most common contiguous sequences of characters in a target text with unused 'placeholder' bytes. The iteration ends when no sequences can be found, leaving the target text effectively compressed. Decompression can be performed by reversing this process, querying known placeholder terms against their corresponding denoted sequence, using a lookup table. In the original paper, this lookup table is encoded and stored alongside the compressed text.";
    int text_size = strlen(text);

    for (int i = 0; i < text_size; ++i) {
        Pair pair = {
            .pair = {text[i], text[i+1]}
        };
        ptrdiff_t i = hmgeti(freq, pair);
        if (i < 0) hmput(freq, pair, 1);
        else freq[i].value += 1;
    }

    for (ptrdiff_t i = 0; i < hmlen(freq); ++i) {
        arrput(freqs_sorted, freq[i]);
    }

    qsort(freqs_sorted, arrlen(freqs_sorted), sizeof(Freq), compare_freqs);

    for (size_t i = 0; i < arrlen(freqs_sorted); ++i) {
        Freq *freqs = &freqs_sorted[i];
        printf("%c%c => %zu\n", freqs->key.pair[0], freqs->key.pair[1], freqs->value);
    }
    return 0;
}
