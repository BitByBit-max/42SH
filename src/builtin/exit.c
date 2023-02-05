#include <stdlib.h>

#include "builtin.h"

int exit_builtin(int code)
{
    exit(code);
}
