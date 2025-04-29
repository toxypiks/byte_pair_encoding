#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>
#include <assert.h>
#include <time.h>

#include <pthread.h>
#include <semaphore.h>

#include "bpe.h"
#include "stb_ds.h"

#define UNREACHABLE(message) \
    do { \
        fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, message); \
        exit(1); \
    } while (0)

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

double get_secs(void)
{
    struct timespec tp = {0};
    int ret = clock_gettime(CLOCK_MONOTONIC, &tp);
    assert(ret == 0);
    return (double)tp.tv_sec + (double)tp.tv_nsec*1e-9;
}

double begin_secs;
#if 1
    #define PROFILE_BEGIN() begin_secs = get_secs();
    #define PROFILE_END(label) printf("%s: %lfsecs\n", (label), get_secs() - begin_secs);
#else
    #define PROFILE_BEGIN(...)
    #define PROFILE_END(...)
#endif

#define ENABLE_THREADS
#define THREAD_COUNT 16
#define REPORT_FREQ 1
#define FREQ_COLLECTION_CHUNK_SIZE (64*1024)
uint32_t *tokens_in = NULL;

size_t tokens_in_cursor = 0;
pthread_mutex_t tokens_in_cursor_mutex = {0};

Freq *freqs[THREAD_COUNT] = {0};
pthread_t threads[THREAD_COUNT] = {0};
sem_t collect_freqs_start = {0};
pthread_barrier_t collect_freqs_stop = {0};

void *freq_collector(void *arg)
{
    size_t id = (size_t)(arg);

    while (true) {
        int ret = sem_wait(&collect_freqs_start);
        if (ret == -1) {
            fprintf(stderr, "ERROR: could not wait on semaphore: %s\n", strerror(errno));
            exit(1);
        }
        hmfree(freqs[id]);
        while (true) {
            size_t begin ,end;
            pthread_mutex_lock(&tokens_in_cursor_mutex);
            if (tokens_in_cursor + FREQ_COLLECTION_CHUNK_SIZE <= arrlen(tokens_in)) {
                begin = tokens_in_cursor;
                tokens_in_cursor += FREQ_COLLECTION_CHUNK_SIZE;
                end = tokens_in_cursor;
            } else {
                begin = tokens_in_cursor;
                tokens_in_cursor = arrlen(tokens_in);
                end = tokens_in_cursor;
            }
            if (end <= begin) {
                pthread_mutex_unlock(&tokens_in_cursor_mutex);
                break;
            }
            pthread_mutex_unlock(&tokens_in_cursor_mutex);

            for (size_t i = begin; i < end; ++i) {
                if (i + 1 >= arrlen(tokens_in)) break;
                Pair pair = {
                    .l = tokens_in[i],
                    .r = tokens_in[i + 1]
                };
                ptrdiff_t place = hmgeti(freqs[id], pair);
                if (place < 0) hmput(freqs[id], pair, 1);
                else freqs[id][place].value += 1;
            }
        }
        pthread_barrier_wait(&collect_freqs_stop);
    }
    UNREACHABLE("freq_collector");
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

    Freq *merged_freq = NULL;
    Pair *pairs = NULL;

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

  #ifdef ENABLE_THREADS

    int ret = pthread_mutex_init(&tokens_in_cursor_mutex, NULL);
    if (ret != 0) {
        fprintf(stderr, "ERROR: could not initialize tokens_in_cursor_mutex: %s\n", strerror(ret));
        return 1;
    }
    ret = sem_init(&collect_freqs_start, 0, 0);
    if (ret != 0) {
        fprintf(stderr, "ERROR: could not initialize collect_freqs_start: %s\n", strerror(ret));
        return 1;
    }

    ret = pthread_barrier_init(&collect_freqs_stop, NULL, THREAD_COUNT + 1);
    if (ret != 0) {
        fprintf(stderr, "ERROR: could not initialize collect_freqs_stop: %s\n", strerror(ret));
        return 1;
    }

    for (size_t id = 0; id < THREAD_COUNT; ++id) {
        ret = pthread_create(&threads[id], NULL, freq_collector, (void*)id);
        if (ret != 0) {
            fprintf(stderr, "ERROR: could not create thread: %s\n", strerror(ret));
            return 1;
        }
    }
#endif // ENABLE_THREADS

    PROFILE_BEGIN();
            //hmfree(merged_freq);
#ifndef ENABLE_THREADS

// Put two chars of token_in into Pair and put in Hashmap if not already there, if there increment counter for pair in hashmap (value)
    for (size_t i = 0; i < arrlen(tokens_in) - 1; ++i) {
        Pair pair = {
            .l = tokens_in[i],
            .r = tokens_in[i+1]
        };
        ptrdiff_t place = hmgeti(merged_freq, pair);
        if (place < 0) hmput(merged_freq, pair, 1);
        else merged_freq[place].value += 1;
    }
#else
    pthread_mutex_lock(&tokens_in_cursor_mutex);
    tokens_in_cursor = 0;
    pthread_mutex_unlock(&tokens_in_cursor_mutex);

    for(size_t i = 0; i < THREAD_COUNT; ++i) sem_post(&collect_freqs_start);
    pthread_barrier_wait(&collect_freqs_stop);

    for(size_t id = 0; id < THREAD_COUNT; ++id) {
        size_t n = hmlen(freqs[id]);
        for (size_t i = 0; i < n; ++i) {
            Pair key = freqs[id][i].key;
            ptrdiff_t place = hmgeti(merged_freq, key);
            if (place < 0) hmputs(merged_freq, freqs[id][i]);
            else merged_freq[place].value += freqs[id][i].value;
        }
    }

#endif
    PROFILE_END("Collecting stats\n");

    size_t iteration = 0;
    for (; iteration < 1; ++iteration) {
        if (iteration%REPORT_FREQ == 0) report_progress(iteration, tokens_in, pairs);

        PROFILE_BEGIN();

        // Find index of pair with max occurence in hashmap
        ptrdiff_t max_index = 0;
        for (ptrdiff_t i = 1; i < hmlen(merged_freq); ++i) {
            if (merged_freq[i].value > merged_freq[max_index].value) {
                max_index = i;
            }
        }
        PROFILE_END("Finding most frequent pair\n");

        if (merged_freq[max_index].value <= 1) break; // No further compression can be done

        // Put pair with max occurence in pairs array at new index (first time its index 256)
        Pair max_pair = merged_freq[max_index].key;
        uint32_t max_token = arrlen(pairs);
        arrput(pairs, max_pair);

        PROFILE_BEGIN();

        // clean up tokens_out for next compressing iteration
        arrsetlen(tokens_out, 0);

        // find pair with max occurence in tokens_in and replace it with token and put it into tokens_out otherwise just put pairs into tokens_out
        for (size_t i = 0; i < arrlen(tokens_in);) {
            if (i + 1 >= arrlen(tokens_in)) {
                arrput(tokens_out, tokens_in[i]);
                i += 1;
            } else {
                Pair pair = {.l = tokens_in[i], .r = tokens_in[i + 1]};
                if (memcmp(&pair, &max_pair, sizeof(pair)) == 0) {
                    ptrdiff_t place;
                    //left part
                    if (arrlen(tokens_out) > 0) {
                        pair.l = tokens_out[arrlen(tokens_out) - 1];
                        pair.r = tokens_in[i];
                        place = hmgeti(merged_freq, pair);
                        assert(place >= 0);
                        assert(merged_freq[place].value > 0);
                        merged_freq[place].value -= 1;

                        pair.r = max_token;
                        place = hmgeti(merged_freq, pair);
                        if (place < 0) hmput(merged_freq, pair, 1);
                        else merged_freq[place].value += 1;
                    }
                    //middle part
                    place = hmgeti(merged_freq, max_pair);
                    assert(place >= 0);
                    assert(merged_freq[place].value > 0);
                    merged_freq[place].value -= 1;
                    arrput(tokens_out, max_token);
                    i += 2;

                    //right part
                    if (i >= arrlen(tokens_in)) {
                        pair.r = tokens_in[i];

                        pair.l = tokens_in[i - 1];
                        place = hmgeti(merged_freq, pair);
                        assert(place >= 0);
                        assert(merged_freq[place].value > 0);
                        merged_freq[place].value -= 1;

                        pair.l = tokens_out[arrlen(tokens_out) - 1];
                        ptrdiff_t place = hmgeti(merged_freq, pair);
                        if (place < 0) hmput(merged_freq, pair, 1);
                        else merged_freq[place].value += 1;
                    }
                } else {
                    arrput(tokens_out, tokens_in[i]);
                    i += 1;
                }
            }
        }
        PROFILE_END("Replacing most frequent pair\n");

        swap(uint32_t*, tokens_in, tokens_out);
    }
    report_progress(iteration, tokens_in, pairs);

    if(dump_pairs(output_file_path, pairs)) return 1;
    printf("INFO: generated %s\n", output_file_path);

   return 0;
}
