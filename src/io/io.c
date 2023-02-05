#include "io.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static FILE *fd;
static int peek = -2;

int io_init(enum option opt, char *str)
{
    peek = -2;
    if (opt == FIL)
    {
        if (!str || str[0] == '\0')
            return 0;
        fd = fopen(str, "r");
    }
    else if (opt == STR)
    {
        if (!str || str[0] == '\0')
            return 0;
        fd = fmemopen(str, strlen(str), "r");
    }
    else if (opt == STD)
        fd = stdin;
    if (!fd)
        return 0;
    return 1;
}

int io_generate(void)
{
    if (peek == -2)
        peek = fgetc(fd);
    return peek;
}

int io_eat(void)
{
    int c = io_generate();
    if (c != EOF)
        peek = -2;
    return c;
}

void io_end(void)
{
    fclose(fd);
}

FILE *io_get_fd(void)
{
    return fd;
}

void io_set_fd(FILE *f)
{
    peek = -2;
    fd = f;
}
