#ifndef PARSER_H
#define PARSER_H

#include "../ast/ast.h"
#include "../lexer/lexer.h"
#include "../token/token.h"
#include "../varmap/varmap.h"

enum parser_status
{
    DONE = 0,
    NO_MATCH,
    ERROR
};

struct ast_node *build(int *error);

/* PARSE ASSIGNMENT WORD */
void parse_ass_word(char *ass, char **name, char **value);
void parse_subshell(char *ass, char **subshell, char **toreplace, int dollar);

#endif /* ! PARSER_H */
