#include "special_vars.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "varmap.h"

char *my_itoa(int value)
{
    int len = 1;
    int n = value;
    int enter = 0;
    while (value > 0)
    {
        value = value / 10;
        len++;
        enter = 1;
    }
    if (enter)
        len--;
    char *str = malloc((len + 1) * sizeof(char));
    str[len] = '\0';
    for (int i = len - 1; i >= 0; i--)
    {
        char c = (n % 10) + '0';
        str[i] = c;
        n = n / 10;
    }
    return str;
}

static void add_09_args(struct varmap *vars, int argc, char *argv[], int ind)
{
    int add = ind == 0 ? 1 : 0; // so that we set variable 1 to 1 and not 0

    for (int i = ind; i < argc; i++)
    {
        char *str = my_itoa(i + add);
        char *a = argv[i];
        varmap_insert(vars, str, a);
        free(str);
    }
}

static void add_nb_args(struct varmap *vars, int argc)
{
    char *str = my_itoa(argc);
    varmap_insert(vars, "#", str);
    free(str);
}

static void add_array_args(struct varmap *vars, int argc, char *argv[])
{
    if (argc == 1)
        return;

    char *res = calloc(strlen(argv[1]) + 1, sizeof(char));
    size_t curr = 0;
    size_t currlen = strlen(argv[1]) + 1;

    for (size_t j = 0; j < strlen(argv[1]); j++, curr++)
        res[curr] = argv[1][j];

    for (int i = 2; i < argc; i++)
    {
        currlen += strlen(argv[i]) + 1;
        res = realloc(res, sizeof(char) * currlen);
        res[curr++] = ' ';
        for (size_t j = 0; j < strlen(argv[i]); j++, curr++)
            res[curr] = argv[i][j];
    }

    res[curr] = '\0';
    varmap_insert(vars, "@", res);
    varmap_insert(vars, "*", res);
    free(res);
}

static void add_uid(struct varmap *vars)
{
    char *str = my_itoa(getuid());
    varmap_insert(vars, "UID", str);
    free(str);
}

void add_pid(struct varmap *vars)
{
    char *str = my_itoa(getpid());
    varmap_insert(vars, "$", str);
    free(str);
}

static void add_script_name(struct varmap *vars, char *name)
{
    char *str = my_itoa(0);
    varmap_insert(vars, str, name);
    free(str);
}

void add_variables(struct varmap *vars, int argc, char *argv[])
{
    // uid
    add_uid(vars);

    if (argc == 0)
        return;

    add_script_name(vars, argv[0]);

    // args $0 - $9
    add_09_args(vars, argc, argv, 1);

    // nb args $#
    add_nb_args(vars, argc);

    // $@ + $*
    add_array_args(vars, argc, argv);
}

void add_variables_func(struct varmap *vars, int argc, char *argv[], char *name)
{
    // uid
    add_uid(vars);

    if (argc == 0)
        return;

    add_script_name(vars, name);

    // args $0 - $9
    add_09_args(vars, argc, argv, 0);

    // nb args $#
    add_nb_args(vars, argc);

    // $@ + $*
    add_array_args(vars, argc, argv);
}
