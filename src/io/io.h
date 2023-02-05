#ifndef IO_H
#define IO_H

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

enum option
{
    FIL,
    STR,
    STD
};

int io_init(enum option opt, char *str);
int io_generate(void);
int io_eat(void);
void io_end(void);
FILE *io_get_fd(void);
void io_set_fd(FILE *f);

#endif /* ! IO_H */
