#ifndef TOKEN_H
#define TOKEN_H

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

enum option_t
{
    WRD,
    IO,
    ASS,
    NOT_WRD,
    NOT_IO,
    NOT_ASS,
};

enum type_t
{
    IF, // 0
    THEN, // 1
    ELIF, // 2
    ELSE, // 3
    FI, // 4
    SCOL, // 5
    NEWL, // 6
    ASS_WORD, // 7
    WORD, // 8
    IONUMBER, // 9
    NEGATION, // 10
    REDIR_MORE, // 11 >
    REDIR_LESS, // 12 <
    REDIR_DMORE, // 13 >>
    REDIR_MOREAND, // 14 >&
    REDIR_LESSAND, // 15 <&
    REDIR_MOREOR, // 16 >|
    REDIR_LESSMORE, // 17 <>
    TOKEN_WHILE, // 18
    TOKEN_UNTIL, // 19
    TOKEN_FOR, // 20
    TOKEN_PIPE, // 21
    TOKEN_OR, // 22
    TOKEN_AND, // 23
    TOKEN_DO, // 24
    TOKEN_DONE, // 25
    TOKEN_IN, // 26
    OPENED_P, // 27
    CLOSED_P, // 28
    OPEN_CB, // 29
    CLOSED_CB, // 30
    CASE, // 31
    ESAC, // 32
    DOUBLE_SCOL, // 33
    AMPERSAND, // 34
    END, // 35
};

struct token
{
    char *data;
    enum type_t type;
};

struct token *token_init(void);
void token_destroy(struct token *t);

#endif /* ! TOKEN_H */
