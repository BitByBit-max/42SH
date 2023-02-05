#include "log.h"

#include <string.h>

static FILE *fd = NULL;

FILE *set_up_log(char *log_file)
{
    if (fd)
        close_file();
    if (strcmp(log_file, "std_out"))
        fd = fopen(log_file, "w+");
    else
        fd = stdout;
    return fd;
}

void close_file(void)
{
    if (fd && fd != stdout)
    {
        fclose(fd);
        fd = NULL;
    }
}

static char *get_type(int type)
{
    char *log_jump_table[] = {
        "IF",           "THEN",           "ELIF",          "ELSE",
        "FI",           "SCOL",           "NEWL",          "ASS_WORD",
        "WORD",         "IONUMBER",       "NEGATION",      "REDIR_MORE",
        "REDIR_LESS",   "REDIR_DMORE",    "REDIR_MOREAND", "REDIR_LESSAND",
        "REDIR_MOREOR", "REDIR_LESSMORE", "TOKEN_WHILE",   "TOKEN_UNTIL",
        "TOKEN_FOR",    "TOKEN_PIPE",     "TOKEN_OR",      "TOKEN_AND",
        "TOKEN_DO",     "TOKEN_DONE",     "TOKEN_IN",      "OPENED_P",
        "CLOSED_P",     "OPEN_CB",        "CLOSED_CB",     "CASE",
        "ESAC",         "DOUBLE_SCOL",    "AMPERSAND",     "END",
    };
    return log_jump_table[type];
}

void log_token(int type, char *data)
{
    if (!fd)
        return;
    char *str_type = get_type(type);
    fprintf(fd, "New token of type %s containing \"%s\" \n", str_type, data);
    fflush(fd);
}

void log_text(char *s)
{
    if (!fd)
        return;
    fprintf(fd, "%s\n", s);
    fflush(fd);
}

void log_int(char *s, int i)
{
    if (!fd)
        return;
    fprintf(fd, "%s %i\n", s, i);
    fflush(fd);
}

void log_char(char *s, char c)
{
    if (!fd)
        return;
    fprintf(fd, "%s %c\n", s, c);
    fflush(fd);
}

void log_2int(char *s1, int i1, char *s2, int i2)
{
    if (!fd)
        return;
    fprintf(fd, "%s %i. %s %i.\n", s1, i1, s2, i2);
    fflush(fd);
}
