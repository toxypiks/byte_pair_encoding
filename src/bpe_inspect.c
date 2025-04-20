#include <stdio.h>
#include "bpe.h"

#include "stb_ds.h"

void usage(const char *program_name)
{
    fprintf(stderr, "Usage %s <input.bpe>\n", program_name);
}

void render_token(Pair *pairs, uint32_t token, char **token_out)
{
    if (token == pairs[token].l) {
        arrput(*token_out, (char)token);
        int arr_last = arrlen(*token_out)-1;
        return;
    }
    render_token(pairs, pairs[token].l, token_out);
    render_token(pairs, pairs[token].r, token_out);
}

int main(int argc, char **argv)
{
    const char* program_name = argv[0];

    if (argc <= 1) {
        usage(program_name);
        fprintf(stderr, "ERROR: no input is provided\n");
    }
    const char* input_file_path = argv[1];

    Pair *pairs = NULL;
    if(!load_pairs(input_file_path, &pairs)) return 1;

    char *tokens_out = NULL;

    print_pairs(&pairs);
    for (uint32_t token = 0; token < arrlen(pairs); ++token) {
        arrsetlen(tokens_out, 0);
        render_token(pairs, token, &tokens_out);
        arrput(tokens_out, 0);
        printf("%zu => %s\n", token, tokens_out);
    }
    arrfree(pairs);
    arrfree(tokens_out);

    return 0;
}
