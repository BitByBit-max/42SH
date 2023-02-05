#include "builtin.h"

#include <string.h>

#include "../vector/vector.h"

struct builtins_str
{
    char *str;
    enum builtins token;
};

enum builtins is_builtin(char *cmd)
{
    struct builtins_str arr[] = {
        { "true", TRUE },         { "false", FALSE }, { "echo", ECHO },
        { "exit", EXIT },         { "cd", CD },       { "export", EXPORT },
        { "continue", CONTINUE }, { "break", BREAK }, { ".", DOT },
        { "unset", UNSET },       { "alias", ALIAS }, { "unalias", UNALIAS }
    };
    for (size_t i = 0; i < 12; i++)
    {
        if (strcmp(cmd, arr[i].str) == 0)
            return arr[i].token;
    }
    return NONE;
}
