#include <dirent.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../varmap/special_vars.h"
#include "builtin.h"

static int dir_exists(char *path)
{
    struct stat s;
    if (path[0] == '~')
    {
        struct passwd *pw = getpwuid(geteuid());
        char *home = pw->pw_dir;
        char newpath[1024];
        snprintf(newpath, sizeof(newpath), "%s%s", home, &path[1]);
        if (stat(newpath, &s) == 0)
        {
            if (S_ISDIR(s.st_mode))
                return 1;
        }
    }
    else if (stat(path, &s) == 0)
    {
        if (S_ISDIR(s.st_mode))
            return 1;
    }
    return 0;
}

static void change_dir(char *path)
{
    char *oldpwd = getenv("PWD");
    char cwd[1024] = { 0 };
    char newpath[1024] = { 0 };
    if (path[0] == '~')
    {
        struct passwd *pw = getpwuid(geteuid());
        char *home = pw->pw_dir;
        snprintf(newpath, sizeof(newpath), "%s%s", home, &path[1]);
        path = newpath;
    }
    else if (path[0] == '.' && path[1] == '/')
    {
        path = strcat(oldpwd, path);
    }
    setenv("OLDPWD", oldpwd, 1);
    chdir(path);
    getcwd(cwd, sizeof(cwd));
    setenv("PWD", cwd, 1);
}

int cd_builtin(struct varmap *vars, char **args,
               size_t size) // fix ~ and symlink
{
    if (size == 0)
    {
        struct passwd *pw = getpwuid(geteuid());
        char *home = pw->pw_dir;
        change_dir(home);
        return 0;
    }
    if (size > 1 || !dir_exists(args[0]))
    {
        fprintf(stderr, "cd: no such file or directory\n");
        int error_code = 1;
        char *error_code_str = my_itoa(error_code);
        varmap_insert(vars, "?", error_code_str);
        free(error_code_str);
        return error_code;
    }
    change_dir(args[0]);
    return 0;
}
