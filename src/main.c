#include <pwd.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "aliasmap/aliasmap.h"
#include "error_msg/error_msg.h"
#include "funcmap/funcmap.h"
#include "io/io.h"
#include "log/log.h"
#include "run/run.h"
#include "varmap/special_vars.h"
#include "varmap/varmap.h"

#define MAX_STR_SIZE 1024

static int pretty_print = 0;

static struct varmap *vars = NULL;

static int handle_options(int argc, char *argv[])
{
    add_variables(vars, argc, argv);

    if (argc > 1)
    {
        size_t i = 1;
        if (strcmp(argv[i], "--pretty-print") == 0)
        {
            pretty_print = 1;
            i++;
        }
        if (strcmp(argv[i], "-c") == 0)
        {
            if (!io_init(STR, argv[i + 1])) // if string is null
                return 3;
        }
        else // file
        {
            if (!io_init(FIL, argv[i])) // if path is null/file doesn't exist
                return 2;
        }
    }
    else
    {
        char input[MAX_STR_SIZE];
        if (!io_init(STD, input))
            return 0;
        return 1; // means continue the loop
    }
    return 1; // keep the loop going, added by elsa from dementor1 debug
}

int main(int argc, char *argv[])
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    setenv("PWD", cwd, 1);

    aliasmap_init(8);
    funcmap_init(8);
    vars = varmap_init(8);
    char IFS[4] = " \t\n\0";
    varmap_insert(vars, "IFS", IFS);
    set_up_log("log_file");

    int exitcode = 0;
    if ((exitcode = handle_options(argc, argv)) == 2)
        return error_io(vars);
    if (exitcode == 3)
        return empty_str_io(vars);

    exitcode = run(vars, pretty_print);

    io_end();
    close_file();
    varmap_destroy(vars);
    funcmap_destroy();
    aliasmap_destroy();
    return exitcode;
}
