#include <stdio.h>

#include "../aliasmap/aliasmap.h"

struct Options
{
    int a_opt;
    size_t ind;
};

static void get_options(char **args, size_t nb, struct Options *opt)
{
    size_t i = 0;
    while (i < nb && args[i][0] == '-')
    {
        char *arg = args[i];
        size_t j = 1;
        if (arg[j] == 'a')
            opt->a_opt = 1;
        else
            break;
        i += 1;
    }
    opt->ind = i;
}

static int remove_all(void)
{
    aliasmap_clear();
    return 0;
}

static int unalias_aux(char **args, size_t nb)
{
    int exitcode = 0;
    for (size_t i = 0; i < nb; i++)
    {
        if (!aliasmap_get(args[i]))
        {
            fprintf(stderr, "%s: not found\n", args[i]);
            exitcode = 1;
            continue;
        }
        aliasmap_remove(args[i]);
    }
    return exitcode;
}

int unalias_builtin(char **args, size_t nb)
{
    struct Options opt;
    opt.a_opt = 0;
    opt.ind = 0;
    get_options(args, nb, &opt);
    if (opt.a_opt)
        return remove_all();
    return unalias_aux(args, nb);
}
