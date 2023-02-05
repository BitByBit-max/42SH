#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../ast/ast.h"
#include "../io/io.h"
#include "../log/log.h"
#include "../run/run.h"
#include "../varmap/special_vars.h"
#include "../varmap/varmap.h"
#include "builtin.h"

static int file_exists(const char *path)
{
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

char *get_dynamic_path(char buffer[2048], char *to_add)
{
    size_t buffer_len = strlen(buffer);
    size_t to_add_len = strlen(to_add);
    char *allocated_path = calloc(buffer_len + to_add_len - 1, sizeof(char));

    int i = 0;
    for (i = 0; buffer[i]; i++)
        allocated_path[i] = buffer[i];
    allocated_path[i] = '/';
    i++;
    int j = 0;
    for (; to_add[j]; i++, j++)
        allocated_path[i] = to_add[j];
    return allocated_path;
}

static char *get_path(char *filename)
{
    char buffer[2048];
    if (!getcwd(buffer, sizeof(buffer)))
        return NULL;
    char *path = get_dynamic_path(buffer, filename);
    if (!path)
        return NULL;
    if (file_exists(path))
        return path;
    if (path)
        free(path);
    return NULL;
}

static int execute_file(char *path)
{
    struct varmap *vars = varmap_init(8);
    add_pid(vars);
    set_up_log("log_file");
    FILE *fd = io_get_fd();

    int exitcode = io_init(FIL, path);
    if (!exitcode)
        return 127;

    exitcode = run(vars, 0);

    close_file();
    varmap_destroy(vars);

    io_end();

    io_set_fd(fd);
    return exitcode;
}

int dot_builtin(char **args, size_t size)
{
    if (size == 0)
    {
        // TO ASK: if no parameter, print usage on stderr or stdout
        fprintf(stderr, ".: filename argument required\n");
        return 127;
    }

    if (!file_exists(args[0]))
    {
        char *path = get_path(args[0]);
        if (!path)
        {
            fprintf(stderr, ".: filename argument required\n");
            return 127;
        }
        int err = execute_file(path);
        free(path);
        return err;
    }

    return execute_file(args[0]);
}
