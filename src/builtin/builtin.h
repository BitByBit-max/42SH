#ifndef BUILTIN_H
#define BUILTIN_H

#include <stddef.h>
#include <string.h>

#include "../varmap/varmap.h"
#include "../vector/vector.h"

enum builtins
{
    TRUE = 0,
    FALSE,
    ECHO,
    EXIT,
    CD,
    EXPORT,
    CONTINUE,
    BREAK,
    DOT,
    UNSET,
    ALIAS,
    UNALIAS,
    NONE
};

/* -------------------------- FUNCTIONS TO EXPORT -------------------------- */

/* RETURNS -1 if cmd is not builtin else nb in enum */
enum builtins is_builtin(char *cmd);

int echo_builtin(char **args, size_t size);

int exit_builtin(int code);

int cd_builtin(struct varmap *vars, char **args, size_t size);

int export_builtin(char **args, size_t size);

int dot_builtin(char **args, size_t size);

int unset_builtin(struct varmap *vars, char **args, size_t size);

int alias_builtin(char **args, size_t nb);

int unalias_builtin(char **args, size_t nb);

#endif /* ! BUILTIN_H */
