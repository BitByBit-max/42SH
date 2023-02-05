#include "lexer.h"

#include <ctype.h>
#include <string.h>

#include "../aliasmap/aliasmap.h"
#include "../io/io.h"
#include "../log/log.h"
#include "../string_t/string_t.h"
#include "../token/token.h"

static FILE *old_fd = NULL;
static int error = 0;

static void skip_to_newl(void)
{
    char c = io_generate();
    while (c != '\n' && c != EOF)
    {
        io_eat();
        c = io_generate();
    }
}

static void skip_spaces(void)
{
    char c = io_generate();
    while (isblank(c))
    {
        io_eat();
        c = io_generate();
    }
}

static struct string *quoting(int ch, struct string *input)
{
    string_append(input, io_eat());
    int c = io_generate();
    while (c != ch && c != EOF) // is EOF needed?(it crashes bash)
    {
        if (ch == '\"' && c == '\\')
        {
            int tmp = c;
            string_append(input, io_eat());
            c = io_generate();
            if (c == '\"' || c == '\\' || c == '`' || c == '\n' || c == '$')
            {
                string_append(input, c);
                io_eat();
            }
            else
                string_append(input, tmp);
        }
        else
            string_append(input, io_eat());

        c = io_generate();
    }
    if (c != ch)
        error = 2;
    if (c != EOF && c != '\n')
        string_append(input, io_eat());
    return input;
}

static void escape(struct string *input) // right ?
{
    string_append(input, io_eat());
    string_append(input, io_eat());
}

static int is_equal(char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '=') // might have to look at more than just that
            return 1;
    }
    return 0;
}

static int is_separator(int c)
{
    if (c == ';' || c == '\n' || c == EOF || c == '|' || c == '>' || c == '<'
        || c == '&' || c == '(' || c == ')')
        return 1;
    return 0;
}

static int is_space(int c)
{
    int res = 0;
    if (c == ' ' || c == '\t')
        res = 1;
    return res;
}

static struct token *handle_word(struct token *t, struct string *input)
{
    t->type = WORD;
    string_append(input, '\0');
    t->data = input->data;
    return t;
}

static enum type_t type_more(int c)
{
    if (c == '>')
    {
        io_eat(); // deal with that
        c = io_generate();
        if (c == '>')
            return REDIR_DMORE;
        else if (c == '&')
            return REDIR_MOREAND;
        else if (c == '|')
            return REDIR_MOREOR;
        else
            return REDIR_MORE;
    }
    return 0;
}

static enum type_t type_less(int c)
{
    if (c == '<')
    {
        io_eat(); // deal with that
        c = io_generate();
        if (c == '>')
            return REDIR_LESSMORE;
        else if (c == '&')
            return REDIR_LESSAND;
        else
            return REDIR_LESS;
    }
    return 0;
}

static enum type_t type_and(int c)
{
    if (c == '&')
    {
        io_eat();
        c = io_generate();
        if (c == '&')
            return TOKEN_AND;
        else
            return AMPERSAND;
    }
    return 0;
}

static enum type_t type_or(int c)
{
    if (c == '|')
    {
        io_eat(); // deal with that
        c = io_generate();
        if (c == '|')
            return TOKEN_OR;
        else
            return TOKEN_PIPE;
    }
    return 0;
}

static enum type_t type_scol(int c)
{
    if (c == ';')
    {
        io_eat(); // deal with that
        c = io_generate();
        if (c == ';')
            return DOUBLE_SCOL;
        else
            return SCOL;
    }
    return 0;
}

enum type_t what_type(int c)
// EOF, \n, ;, ;, |, >, <, >>, >&, <&, >|, <>, ||, &&
{
    enum type_t type;
    type = type_more(c);
    if (!type)
        type = type_less(c);
    if (!type)
        type = type_and(c);
    if (!type)
        type = type_or(c);
    if (!type)
        type = type_scol(c);
    switch (c)
    {
    case EOF:
        type = END;
        break;
    case '\n':
        type = NEWL;
        break;
    case '(':
        type = OPENED_P;
        break;
    case ')':
        type = CLOSED_P;
        break;
    default:
        break;
    }
    return type;
}

static struct token *handle_t(struct token *t, struct string *input)
{
    t->type = what_type(io_generate());
    if (t->type == WORD)
        return t;
    string_append(input, '\0');
    t->data = input->data;
    int c = io_generate();
    if (t->type != REDIR_LESS && t->type != REDIR_MORE && t->type != TOKEN_PIPE
        && t->type != SCOL && t->type != AMPERSAND)
        io_eat();
    c++;
    log_token(t->type, t->data);
    return t;
}

static void handle_subshell(struct string *input)
{
    string_append(input, io_eat());
    int c = io_generate();
    while (c != ')' && c != EOF) // while we don't see the end of paranthesis
    {
        string_append(input, c); // we add the chars into the word
        io_eat();
        c = io_generate();
    }
    io_eat();
    string_append(input, c); // now we add the )
}

struct token *build_token(struct token *t, struct string *input)
{
    while (1 /*token not done*/)
    {
        int c = io_generate();
        if (is_space(c))
        {
            if (input->size > 0)
                return handle_word(t, input);
            skip_spaces();
            continue;
        }
        if (c == '#' && input->size == 0)
        {
            skip_to_newl();
            continue;
        }
        if (c == '\'' || c == '\"' || c == '`') // we can't \ in quotes right?
        {
            input = quoting(c, input);
            continue;
        }
        if (c == '\\')
        {
            escape(input);
            continue;
        }
        if (is_separator(c)) // EOF, \n, ;, |, >, <, &&, ||, (, )...
        {
            if (c == '(' && input->size > 0
                && input->data[input->size - 1] == '$')
            {
                handle_subshell(input);
                continue;
            }
            if (input->size > 0)
                return handle_word(t, input);
            t = handle_t(t, input);
            if (t->type != WORD)
                return t;
        }
        string_append(input, io_eat());
    }
    string_append(input, '\0');
    t->data = input->data;
    t->type = WORD;
    return t;
}

static enum type_t lex_reserved(char *str)
{
    if (strcmp(str, "if") == 0)
        return IF;
    else if (strcmp(str, "then") == 0)
        return THEN;
    else if (strcmp(str, "elif") == 0)
        return ELIF;
    else if (strcmp(str, "else") == 0)
        return ELSE;
    else if (strcmp(str, "fi") == 0)
        return FI;
    else if (strcmp(str, "while") == 0)
        return TOKEN_WHILE;
    else if (strcmp(str, "until") == 0)
        return TOKEN_UNTIL;
    else if (strcmp(str, "for") == 0)
        return TOKEN_FOR;
    else if (strcmp(str, "do") == 0)
        return TOKEN_DO;
    else if (strcmp(str, "done") == 0)
        return TOKEN_DONE;
    else if (strcmp(str, "in") == 0)
        return TOKEN_IN;
    else if (strcmp(str, "!") == 0)
        return NEGATION;
    else if (strcmp(str, "case") == 0)
        return CASE;
    else if (strcmp(str, "esac") == 0)
        return ESAC;
    else if (strcmp(str, "{") == 0)
        return OPEN_CB;
    else if (strcmp(str, "}") == 0)
        return CLOSED_CB;
    return WORD;
}

static enum type_t lex_io(char *str)
{
    int i = 0;
    for (; str[i] != '\0'; i++)
    {
        char c = str[i];
        if ('0' > c || c > '9')
            break;
    }
    int c = io_generate();
    return (str[i] == '\0' && (c == '>' || c == '<')) ? IONUMBER : WORD;
}

static enum type_t lex_variable(char *str)
{
    if (is_equal(str))
        return ASS_WORD;
    return WORD;
}

static size_t cnt = 0;

static int starts_with_self(char *str1, char *str2)
{
    int res = strncmp(str1, str2, strlen(str2)) == 0 && cnt > 0;
    cnt -= res;
    return res;
}

static struct token *lex_alias(struct string *input)
{
    char *value = aliasmap_get(input->data);
    if (!value || strcmp(value, input->data) == 0
        || starts_with_self(value, input->data))
        return NULL;
    cnt += 1;

    old_fd = io_get_fd();

    io_init(STR, value);

    struct token *t = lexing(0, 0, 1, &error); // not sure io flag

    return t;
}

struct token *lexing(int restricted, int io, int alias, int *err)
{
    struct token *t = token_init();
    struct string *input = string_init(1);
    t = build_token(t, input);
    if (t->type == END && old_fd)
    {
        io_end();
        io_set_fd(old_fd);
        old_fd = NULL;
        token_destroy(t);
        string_destroy(input);
        return lexing(restricted, io, alias, &error);
    }
    if (t->type == WORD)
    {
        if (!restricted)
        {
            if (alias)
            {
                struct token *new = lex_alias(input);
                if (new)
                {
                    string_destroy(input);
                    token_destroy(t);
                    return new;
                }
            }
            t->type = lex_reserved(t->data);
            if (t->type == WORD)
                t->type = lex_variable(t->data);
        }
        if (io && t->type == WORD)
            t->type = lex_io(t->data);
        log_token(t->type, t->data);
    }
    string_destroy(input);
    *err = error;
    return t;
}
