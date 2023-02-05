#include <stdio.h>
#include <string.h>

#include "builtin.h"

struct Options
{
    int n_opt;
    int e_opt;
    int E_opt;
    size_t ind;
};

static void get_options(char **args, size_t nb, struct Options *opt)
{
    size_t i = 0;
    while (i < nb && args[i][0] == '-')
    {
        char *arg = args[i];
        size_t j = 1;
        if (arg[j] != 'n' && arg[j] != 'e' && arg[j] != 'E')
            break;
        for (; arg[j] == 'n' || arg[j] == 'e' || arg[j] == 'E'; j++)
        {
            switch (arg[j])
            {
            case 'n':
                opt->n_opt = 1;
                break;
            case 'e':
                opt->E_opt = 0;
                opt->e_opt = 1;
                break;
            case 'E':
                opt->e_opt = 0;
                opt->E_opt = 1;
                break;
            default:
                break;
            }
        }
        i += 1;
    }
    opt->ind = i;
}

static int is_escapable(int c)
{
    return c == 'n' || c == 't' || c == '\\';
}

static int print_all(char **args, size_t nb, struct Options opt)
{
    for (size_t i = opt.ind; i < nb; i++)
    {
        char *str = args[i];
        for (size_t j = 0; str[j] != '\0'; j++)
        {
            if (opt.e_opt && str[j] == '\\' && is_escapable(str[j + 1]))
            {
                if (str[j + 1] == 'n')
                    putc('\n', stdout);
                else if (str[j + 1] == 't')
                    putc('\t', stdout);
                else if (str[j + 1] == '\\')
                    putc('\\', stdout);
                else
                    putc(str[j], stdout);
                j += 1;
            }
            else
            {
                putc(str[j], stdout);
            }
        }
        if (i + 1 != nb)
            putc(' ', stdout);
    }
    if (!opt.n_opt)
        putc('\n', stdout);
    fflush(stdout);
    return 0;
}

int echo_builtin(char **args, size_t nb)
{
    if (nb == 0 || strcmp(args[0], "") == 0)
    {
        printf("\n");
        fflush(stdout);
        return 0;
    }
    struct Options opt;
    opt.n_opt = 0;
    opt.e_opt = 0;
    opt.E_opt = 0;
    opt.ind = 0;
    get_options(args, nb, &opt);

    return print_all(args, nb, opt);
}
