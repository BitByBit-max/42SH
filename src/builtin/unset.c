#include <stdio.h>

#include "../funcmap/funcmap.h"
#include "../varmap/varmap.h"
#include "builtin.h"

struct Options
{
    int f_opt;
    int v_opt;
    size_t ind;
};

static struct varmap *vars_g = NULL;

static void get_options(char **args, size_t nb, struct Options *opt)
{
    size_t i = 0;
    while (i < nb && args[i][0] == '-')
    {
        char *arg = args[i];
        size_t j = 1;
        if (arg[j] != 'f' && arg[j] != 'v')
            break;
        while (arg[j] == 'f' || arg[j] == 'v')
        {
            if (arg[j] == 'f')
                opt->f_opt = 1;
            else if (arg[j] == 'v')
                opt->v_opt = 1;
            j += 1;
        }
        i += 1;
    }
    opt->ind = i;
}

static int unset_functions(char **args, size_t size, struct Options opt)
{
    for (size_t i = opt.ind; i < size; i++)
    {
        funcmap_remove(args[i]);
    }
    return 0;
}

static int unset_variables(char **args, size_t size, struct Options opt)
{
    for (size_t i = opt.ind; i < size; i++)
    {
        varmap_remove(vars_g, args[i]);
    }
    return 0;
}

int unset_builtin(struct varmap *vars, char **args, size_t size)
{
    vars_g = vars;
    if (size == 0)
        return 0;
    struct Options opt = { 0, 0, 0 };
    get_options(args, size, &opt);

    if (opt.f_opt)
        return unset_functions(args, size, opt);

    return unset_variables(args, size, opt);
}
