#include "token.h"

#include <stdlib.h>

struct token *token_init(void)
{
    return calloc(1, sizeof(struct token));
}

void token_destroy(struct token *t)
{
    if (!t)
        return;
    free(t->data);
    free(t);
}
