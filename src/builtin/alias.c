#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../aliasmap/aliasmap.h"

static int alias_print(char *aliasname)
{
    char *value = aliasmap_get(aliasname);
    if (!value)
    {
        fprintf(stderr, "%s: not found\n", aliasname);
        return 1;
    }
    printf("%s='%s'\n", aliasname, value);
    return 0;
}

static int contains_equal(char *str)
{
    for (size_t i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '=')
            return 1;
    }
    return 0;
}

static int alias_aux(char **args, size_t nb)
{
    int exitcode = 0;
    for (size_t i = 0; i < nb; i++)
    {
        if (!contains_equal(args[i]))
        {
            int err = alias_print(args[i]);
            if (err != 0)
                exitcode = err;
            continue;
        }

        aliasmap_ass_insert(args[i]);
    }
    return exitcode;
}

int alias_builtin(char **args, size_t nb)
{
    if (nb == 0)
    {
        aliasmap_print();
        return 0;
    }
    return alias_aux(args, nb);
}
