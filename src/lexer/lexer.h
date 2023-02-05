#ifndef LEXER_H
#define LEXER_H

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct token *lexing(int restricted, int io, int alias, int *err);

#endif /* ! LEXER_H */
