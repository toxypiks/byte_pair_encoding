#ifndef BPE_H_
#define BPE_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct Pair {
    uint32_t l, r;
} Pair;

int read_entire_file(const char *file_path, void **data, size_t *data_size);
bool load_pairs(const char *file_path, Pair **pairs);

#endif // BPE_H_
