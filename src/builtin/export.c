#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "builtin.h"

static int is_varname(char c)
{
    return isalpha(c) || isdigit(c) || c == '_';
}

static int parse_vars(char *arg, char **name, char **value)
{
    size_t i = 0;
    size_t len = strlen(arg);
    while (is_varname(arg[i]))
        i += 1;

    if (i == len)
    {
        *name = strdup(arg);
        return 0;
    }
    *name = strndup(arg, i);
    if (arg[i] != '=')
        return 2;
    i += 1;
    size_t start = i;
    while (is_varname(arg[i]))
        i += 1;
    *value = strndup(arg + start, i - start);
    return 0;
}

static int export(char **args, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        if (strcmp(args[i], "=") == 0)
            return 2;
        char *name = NULL;
        char *value = NULL;
        int err = parse_vars(args[i], &name, &value);
        if (err != 0)
        {
            if (name)
                free(name);
            if (value)
                free(value);
            return err;
        }

        if (!value)
            setenv(name, "", 1);
        else
        {
            setenv(name, value, 1);
            free(value);
        }
        free(name);
    }
    return 0;
}

int export_builtin(char **args, size_t size)
{
    if (size == 0)
        return 0;
    int err = export(args, size);
    if (err == 2)
        fprintf(stderr, "error");
    return err;
}
