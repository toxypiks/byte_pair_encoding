#ifndef BPE_H_
#define BPE_H_

#include <stdint.h>

typedef struct Pair {
    uint32_t l, r;
} Pair;

Pair *pairs = NULL;

#endif // BPE_H_
