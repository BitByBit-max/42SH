#include "exec.h"

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../ast/ast.h"
#include "../builtin/builtin.h"
#include "../error_msg/error_msg.h"
#include "../expansion/expansion.h"
#include "../funcmap/funcmap.h"
#include "../io/io.h"
#include "../log/log.h"
#include "../parser/parser.h"
#include "../varmap/special_vars.h"
#include "../varmap/varmap.h"

static int continue_flag = 0;
static int break_flag = 0;
static int exit_flag = 0;

static int exec_builtin_cmd(struct varmap *vars, char **args, size_t nb,
                            enum builtins b)
{
    switch (b)
    {
    case TRUE:
    case FALSE:
        return (int)b;
    case ECHO:
        return echo_builtin(args, nb);
    case EXIT: // exit should only exit and free everything normally
        exit_flag = 1;
        if (nb == 1)
            return exit_builtin(atoi(args[0]));
        return exit_builtin(0);
    case CD:
        return cd_builtin(vars, args, nb);
    case EXPORT:
        return export_builtin(args, nb);
    case CONTINUE:
        continue_flag = 1;
        break;
    case BREAK:
        break_flag = 1;
        return 0;
    case DOT:
        return dot_builtin(args, nb);
    case UNSET:
        return unset_builtin(vars, args, nb);
    case UNALIAS:
        return unalias_builtin(args, nb);
    case ALIAS:
        return alias_builtin(args, nb);
    case NONE:
        return 0; // WILL NEVER HAPPEN
    default:
        return 0;
    }
    return 2;
}

static char **duplicate_args(char **args, size_t nb)
{
    char **res = calloc(nb, sizeof(char *));
    if (!res)
        return NULL;
    for (size_t i = 0; i < nb; i++)
    {
        res[i] = strdup(args[i]);
    }
    return res;
}

static char **remove_empty_strings(char **args, size_t *nb)
{
    size_t new_nb = 0;
    for (size_t i = 0; i < *nb; i++)
    {
        if (args[i][0] != '\0')
            new_nb++;
    }
    if (new_nb == *nb)
        return args;
    // printf("new size is: %lu\n", new_nb);
    char **new_args = (char **)calloc(new_nb, sizeof(char *));
    size_t j = 0;
    for (size_t i = 0; i < *nb; i++)
    {
        if (args[i][0] != '\0')
        {
            new_args[j] = strdup(args[i]);
            j++;
        }
    }
    for (size_t i = 0; i < *nb; i++)
        free(args[i]);
    free(args);
    *nb = new_nb;
    return new_args;
}

static char **expand_args(struct varmap *vars, char **args, size_t *nb,
                          int remove_quotations)
{
    for (size_t i = 0; i < *nb; i++)
    {
        args[i] = expand_var(vars, args[i]);
        args[i] = expand_str(args[i], remove_quotations);
    }
    return args;
}

static void free_args(char **args, size_t nb)
{
    if (!args)
        return;
    for (size_t i = 0; i < nb; i++)
        free(args[i]);
    free(args);
}

static int exec_func_cmd(struct varmap *vars_g, struct func *func, char **args,
                         size_t nb)
{
    if (!func)
        return 2;

    args = expand_args(vars_g, args, &nb, 1); // expand with global vars

    struct varmap *vars = varmap_init(nb + 1); // save vars
    add_variables_func(vars, nb, args, func->name);

    // execute ast
    int exitcode = exec_ast(vars, func->body);

    // destroy varmap
    varmap_destroy(vars);

    return exitcode;
}

static void parse_variables(struct simple_cmd_node *cmd_node)
{
    if (cmd_node->vars)
        return;

    for (struct ast_node *var = cmd_node->vars; var; var = var->next)
    {
        char *name = NULL;
        char *value = NULL;
        if (!var->body.variable || !var->body.variable->assignment)
            continue;
        parse_ass_word(var->body.variable->assignment, &name, &value);
        setenv(name, value, 1);
        free(name);
        free(value);
    }
}

static int contains_subshell(char *ass)
{
    if (ass[0] == '\0')
        return 0;
    if (ass[0] == '`')
        return 2;
    char prev = ass[0];
    for (size_t i = 1; ass[i] != '\0'; i++)
    {
        if (prev != '\\' && ass[i] == '`')
            return 2;
        if (prev == '$' && ass[i] == '(')
            return 1;
        prev = ass[i];
    }
    return 0;
}

static struct ast_node *init_redir_subshell(struct ast_node *tree, char *path)
{
    struct ast_node *redir = ast_node_init(REDIR);
    redir->body.redir->elem = tree;
    redir->body.redir->type = MORE;
    redir->body.redir->src = redir_file_init();
    redir->body.redir->src->fd = 1;
    redir->body.redir->dest = redir_file_init();
    redir->body.redir->dest->path = strdup(path);
    return redir;
}

static int exec_var_subshell_aux(struct varmap *vars, char *str, char **path)
{
    int pid = fork();
    if (pid == 0)
    {
        add_pid(vars);
        set_up_log("var_log_file");
        int exitcode = 0;
        FILE *fd = io_get_fd();

        int error = io_init(STR, str);
        if (!error)
            exit(2); // ?

        struct ast_node *tree = build(&error);
        if (error == 2)
        {
            int exitcode = error_parse(tree, vars);
            exit(exitcode); // ?
        }

        sprintf(*path, ".subshell.%u.tmp", getpid());
        struct ast_node *redir = init_redir_subshell(tree, *path);

        error = exec_ast(vars, redir);

        if (error && exitcode != 1)
        {
            int exitcode = error_exec(redir, vars);
            exit(exitcode); // ?
        }

        close_file();
        ast_node_destroy(redir);

        io_end();

        io_set_fd(fd);
        exit(exitcode); // ?
        // return exitcode;
    }
    else if (pid > 0)
    {
        sprintf(*path, ".subshell.%u.tmp", pid);
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
    else
        return 2;
    return 0;
}

static char *read_file(const char *fileName)
{
    FILE *file = fopen(fileName, "rb");
    if (!file)
        return NULL;
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    if (fileSize == 0)
    {
        fclose(file);
        return NULL;
    }
    rewind(file);
    char *fileContent = (char *)malloc(sizeof(char) * (fileSize + 1));
    if (!fileContent)
    {
        fclose(file);
        return NULL;
    }
    size_t bytesRead = fread(fileContent, sizeof(char), fileSize, file);
    if (bytesRead < fileSize)
    {
        free(fileContent);
        fclose(file);
        return NULL;
    }
    if (fileSize > 0)
        fileContent[fileSize - 1] = '\0';
    else
        fileContent[0] = '\0';
    if (fclose(file) != 0)
    {
        free(fileContent);
        return NULL;
    }
    return fileContent;
}

static char *exec_var_subshell(struct varmap *vars, char *str)
{
    char *path = calloc(1000, sizeof(char));
    int error = exec_var_subshell_aux(vars, str, &path);

    if (error != 0 && error != 1) // ???
    {
        remove(path);
        free(path);
        return NULL;
    }

    char *value = read_file(path);

    remove(path);
    free(path);
    return value;
}

static void handle_simple_cmd_subshells(struct varmap *vars, char **args,
                                        size_t nb)
{
    for (size_t i = 0; i < nb; i++)
    {
        int dollar = 0;
        while ((dollar = contains_subshell(args[i])))
        {
            char *subshell = NULL;
            char *toreplace = NULL;

            parse_subshell(args[i], &subshell, &toreplace, dollar);

            char *value = exec_var_subshell(vars, subshell);
            if (!value)
            {
                value = calloc(1, sizeof(char));
            }

            char *str = args[i];
            args[i] = replace_var(args[i], toreplace, value);

            free(str);
            free(value);
            free(subshell);
            free(toreplace);
        }
    }
}

static int exec_simple_cmd(struct varmap *vars, struct ast_node *node)
{
    struct simple_cmd_node *cmd_node = node->body.simple_cmd;
    char *cmd_name = strdup(cmd_node->cmd);
    size_t nb = cmd_node->args->size;
    char **args = duplicate_args(cmd_node->args->data, nb);

    handle_simple_cmd_subshells(vars, args, nb);

    args = expand_args(vars, args, &nb, 1);

    cmd_name = expand_var(vars, cmd_name);
    cmd_name = expand_str(cmd_name, 1);

    enum builtins res = is_builtin(cmd_name);
    if (res != NONE)
    {
        // args = handle_IFS(args, &nb);
        int exitcode = exec_builtin_cmd(vars, args, nb, res);
        free_args(args, nb);
        free(cmd_name);
        return exitcode;
    }

    struct func *func = funcmap_get(cmd_name);
    if (func)
    {
        int exitcode = exec_func_cmd(vars, func, args, nb);
        free_args(args, nb);
        free(cmd_name);
        return exitcode;
    }

    free(cmd_name);
    free_args(args, nb);

    parse_variables(cmd_node);

    int pid = fork();
    if (pid == 0)
    {
        handle_simple_cmd_subshells(vars, cmd_node->args->data,
                                    cmd_node->args->size);

        args =
            expand_args(vars, cmd_node->args->data, &(cmd_node->args->size), 1);
        // cmd_node->args->data = handle_IFS(cmd_node->args->data,
        // &(cmd_node->args->size));
        // cmd_node->args->capacity *= 4; //TODO: figure out what to do with
        // this because its not resizing correctly on insert
        cmd_node->args =
            vector_insert(cmd_node->args, 0, strdup(cmd_node->cmd));

        cmd_node->args = vector_append(cmd_node->args, NULL);
        execvp(cmd_node->cmd, cmd_node->args->data);
        fprintf(stderr, "%s: command not found\n", cmd_node->cmd);
        exit(127);
    }
    else if (pid > 0)
    {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
    else
    {
        return 2;
    }
}

static int exec_subshell(struct varmap *vars, struct ast_node *subshell_node)
{
    int status = 0;
    int child_pid = fork();
    if (child_pid == 0)
    {
        // FILE *f = io_get_fd();
        // fflush(f);
        status = exec_ast(vars, subshell_node);
        // this is not reached
        exit(status); // it's exiting in child process so should be fine
    }
    else
    {
        int child_status;
        waitpid(child_pid, &child_status, 0);
        status = WEXITSTATUS(child_status);
    }
    return status;
}

static int exec_cmp_list(struct varmap *vars, struct ast_node *node)
{
    struct cmp_list_node *cmp_list = node->body.cmp_list;

    int status = 0;
    for (struct ast_node *cmd = cmp_list->elem; cmd != NULL; cmd = cmd->next)
    {
        // printf("reach here\n");
        if (break_flag)
            break;
        if (cmd->type == CMP_LIST)
        {
            switch (cmd->body.cmp_list->type)
            {
            case CMD_BLOCK:
            case LIST:
                status = exec_ast(vars, cmd);
                break;
            case SUBSHELL:
                status = exec_subshell(vars, cmd);
                break;
            }
        }
        else
            status = exec_ast(vars, cmd);
    }
    return status;
}

static int exec_if_cmd(struct varmap *vars, struct ast_node *node)
{
    struct if_cmd_node *if_cmd = node->body.if_cmd;
    int status = exec_ast(vars, if_cmd->cond);
    if (status == 0)
        return exec_ast(vars, if_cmd->cond_true);
    return exec_ast(vars, if_cmd->cond_false);
}

static int redir_more(struct varmap *vars, struct redir_node *redir)
{
    int fd_dest = open(redir->dest->path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_dest < 0)
    {
        return 2;
    }
    // printf("this is: |%d|\n", redir->src->fd);
    int saved_fd = dup(redir->src->fd);
    dup2(fd_dest, redir->src->fd);
    close(fd_dest);
    exec_ast(vars, redir->elem);
    dup2(saved_fd, redir->src->fd);
    close(saved_fd);
    return 0;
}

static int redir_less(struct varmap *vars, struct redir_node *redir)
{
    int fd = open(redir->dest->path, O_RDONLY);
    if (fd < 0)
    {
        return 2;
    }
    int saved_stdin = dup(STDIN_FILENO);
    dup2(fd, STDIN_FILENO);
    close(fd);
    exec_ast(vars, redir->elem);
    dup2(saved_stdin, STDIN_FILENO);
    close(saved_stdin);
    return 0;
}

static int redir_dmore(struct varmap *vars, struct redir_node *redir)
{
    int fd_dest = open(redir->dest->path, O_CREAT | O_APPEND | O_WRONLY, 0644);
    if (fd_dest < 0)
    {
        return 2;
    }
    int saved_fd = dup(redir->src->fd);
    dup2(fd_dest, redir->src->fd);
    close(fd_dest);
    exec_ast(vars, redir->elem);
    dup2(saved_fd, redir->src->fd);
    close(saved_fd);
    return 0;
}

static int redir_moreand(struct varmap *vars, struct redir_node *redir)
{
    int fd_src = redir->src->fd;
    int fd_dest = redir->dest->fd;
    int saved_fd = dup(fd_dest);
    close(fd_dest);
    dup2(fd_src, fd_dest);
    exec_ast(vars, redir->elem);
    dup2(saved_fd, fd_dest);
    close(saved_fd);
    return 0;
}

static int redir_lessand(struct redir_node *redir)
{
    if (redir->src->fd != -1)
    {
        dup2(redir->src->fd, STDIN_FILENO);
        close(redir->src->fd);
    }
    return 0;
}

static int redir_moreor(struct varmap *vars, struct redir_node *redir)
{
    int fd_dest = open(redir->dest->path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd_dest < 0)
    {
        return 2;
    }
    int saved_fd = dup(redir->src->fd);
    dup2(fd_dest, redir->src->fd);
    close(fd_dest);
    exec_ast(vars, redir->elem);
    dup2(saved_fd, redir->src->fd);
    close(saved_fd);
    return 0;
}

static int redir_lessmore(struct varmap *vars, struct redir_node *redir)
{
    int fd = open(redir->dest->path, O_RDWR | O_CREAT, 0644);
    if (fd < 0)
    {
        return 2;
    }
    int saved_stdin = dup(STDIN_FILENO);
    dup2(fd, STDIN_FILENO);
    close(fd);
    exec_ast(vars, redir->elem);
    dup2(saved_stdin, STDIN_FILENO);
    close(saved_stdin);
    return 0;
}

static int exec_redir(struct varmap *vars, struct ast_node *node)
{
    struct redir_node *redir = node->body.redir;
    int exit_code;
    switch (redir->type)
    {
    case MORE:
        exit_code = redir_more(vars, redir);
        break;
    case LESS:
        exit_code = redir_less(vars, redir);
        break;
    case DMORE:
        exit_code = redir_dmore(vars, redir);
        break;
    case MOREAND:
        exit_code = redir_moreand(vars, redir);
        break;
    case LESSAND:
        exit_code = redir_lessand(redir);
        break;
    case MOREOR:
        exit_code = redir_moreor(vars, redir);
        break;
    case LESSMORE:
        exit_code = redir_lessmore(vars, redir);
        break;
    }
    return exit_code;
}

static int exec_pipe(struct varmap *vars, struct ast_node *node)
{
    struct pipeline_node *pipe_node = node->body.pipe;
    int p[2];
    pid_t pid;
    int status;
    if (pipe_node->elem->next)
    {
        pipe(p);
        if ((pid = fork()) == 0)
        {
            close(p[0]);
            dup2(p[1], STDOUT_FILENO);
            close(p[1]);
            exec_ast(vars, pipe_node->elem);
            exit(0);
        }
        else
        {
            close(p[1]);
            dup2(p[0], STDIN_FILENO);
            close(p[0]);
            waitpid(pid, &status, 0);
            exec_ast(vars, pipe_node->elem->next);
        }
    }
    else
    {
        exec_ast(vars, pipe_node->elem);
    }
    return 0;
}

static int exec_and(struct varmap *vars, struct and_or_node *and)
{
    int status = exec_ast(vars, and->left);
    if (status == 0)
    {
        return exec_ast(vars, and->right);
    }
    return status;
}

static int exec_or(struct varmap *vars, struct and_or_node *or_node)
{
    int status = exec_ast(vars, or_node->left);
    if (status != 0)
    {
        return exec_ast(vars, or_node->right);
    }
    return status;
}

static int exec_and_or(struct varmap *vars, struct ast_node *node)
{
    struct and_or_node *and_or = node->body.and_or;
    if (and_or->type == OR)
        return exec_or(vars, and_or);
    return exec_and(vars, and_or);
}

static int exec_while_loop(struct varmap *vars, struct ast_node *node)
{
    struct loop_node *while_loop = node->body.while_loop;

    int status;
    int exitcode = 0;
    while ((status = exec_ast(vars, while_loop->cond)) == 0)
    {
        exitcode = exec_ast(vars, while_loop->body);
        if (break_flag)
        {
            break_flag = 0;
            break;
        }
        if (continue_flag)
        {
            continue_flag = 0;
            continue;
        }
    }
    return exitcode;
}

static int exec_until_loop(struct varmap *vars, struct ast_node *node)
{
    struct loop_node *loop = node->body.until_loop;
    int exitcode = 0;
    int status = 1;
    while (status != 0)
    {
        if (break_flag)
        {
            break_flag = 0;
            break;
        }
        if (continue_flag)
        {
            continue_flag = 0;
            continue;
        }
        status = exec_ast(vars, loop->cond);
        if (status == 0)
            break;
        exitcode = exec_ast(vars, loop->body);
    }
    return exitcode;
}

/* returns index of match in ifs_matrix if found; else returns -1 */
static int char_in_ifs(char c, char *ifs, int ifs_len)
{
    for (int i = 0; i < ifs_len; i++)
    {
        if (ifs[i] == c)
            return i;
    }
    return -1;
}

static size_t skip_until_char_next(char *str, size_t start_index)
{
    char to_encounter = str[start_index];
    size_t i = start_index + 1;
    for (; str[i]; i++)
    {
        if (str[i] == to_encounter)
            break;
    }
    return i;
}

static size_t count_args_IFS(char **args, size_t *size, char *ifs)
{
    size_t args_count = 0;
    int ifs_len = strlen(ifs);
    for (size_t i = 0; i < *size; i++)
    {
        int found_ifs = 1;
        int starts_with_ifs = -1;
        // printf("|%s|\n", args[i]);
        for (size_t j = 0; args[i][j]; j++)
        {
            if (char_in_ifs(args[i][j], ifs, ifs_len) != -1) // found ifs
            {
                if (starts_with_ifs == -1)
                {
                    starts_with_ifs = 0;
                    args_count++;
                }
                found_ifs = 1;
            }
            else if (args[i][j] == '\"' || args[i][j] == '\'')
            {
                j = skip_until_char_next(args[i], j);
            }
            else
            {
                starts_with_ifs = 0;
                if (found_ifs)
                {
                    found_ifs = 0;
                    args_count++;
                }
            }
        }
        if (starts_with_ifs == -1)
            args_count++;
    }
    return args_count;
}

static int ifs_in_string(char *str, char *ifs, size_t ifs_len)
{
    size_t str_len = strlen(str);
    for (size_t i = 0; i < str_len; i++)
    {
        if (str[i] == '\"' || str[i] == '\'')
            i = skip_until_char_next(str, i);
        for (size_t j = 0; j < ifs_len; j++)
        {
            if (ifs[j] == str[i])
                return 1;
        }
    }
    return 0;
}

// if no ifs pair is found, set it to last index available so ifs_len - 1
static size_t skip_until_IFS_Pair(char *str, char *ifs, size_t j)
{
    size_t len = strlen(str);
    size_t ifs_len = strlen(ifs);
    size_t i = j + 1;
    int found_word = 0;
    for (; i < len; i++)
    {
        if (char_in_ifs(str[i], ifs, ifs_len) == -1)
        {
            found_word = 1;
            break;
        }
    }
    if (!found_word)
        return len - 1;
    return i - 1;
}

static char *remove_IFS_clones(char *str, char *ifs)
{
    size_t str_len = strlen(str);
    char *new_str = calloc(str_len + 1, sizeof(char));
    size_t new_str_index = 0;
    size_t ifs_len = strlen(ifs);
    // printf("str is: |%s|\n", str);
    for (size_t i = 0; i < str_len; i++)
    {
        // printf("here\n");
        new_str[new_str_index] = str[i];
        new_str_index++;
        for (size_t j = 0; j < ifs_len; j++)
        {
            if (str[i] == ifs[j])
            {
                // if find ifs skip until the index before first non ifs index
                i = skip_until_IFS_Pair(str, ifs, i);
                break;
            }
        }
    }
    // printf("reacher\n");
    free(str);
    // printf("new is: %s\n", new_str);
    return new_str;
}

static char **handle_IFS(char **args, size_t *size, char *ifs,
                         size_t args_count)
{
    if (args_count == 0)
        return args;
    char **new_args = (char **)malloc(args_count * sizeof(char *));
    size_t i = 0;
    size_t j = 0;
    size_t j_start = 0;
    size_t new_args_index = 0;
    for (; i < *size; i++)
    {
        char *str = args[i];
        size_t len = strlen(str);
        size_t ifs_len = strlen(ifs);
        if (!ifs_in_string(str, ifs, ifs_len))
        {
            new_args[new_args_index] = strdup(str);
            new_args[new_args_index] =
                remove_IFS_clones(new_args[new_args_index], ifs);
            new_args_index++;
        }
        else
        {
            j = 0;
            j_start = 0;
            for (; j < len; j++)
            {
                if (char_in_ifs(str[j], ifs, ifs_len) != -1) // found ifs
                {
                    // printf("j_start is: %lu | j is: %lu\n", j_start, j);
                    size_t len = j - j_start;
                    // printf("len is: %lu\n", len);
                    new_args[new_args_index] =
                        (char *)(calloc(len + 1, sizeof(char)));
                    strncpy(new_args[new_args_index], &(args[i][j_start]), len);
                    new_args_index++;
                    j = skip_until_IFS_Pair(str, ifs, j);
                    j_start = j + 1;
                    // printf("end of if and now j is: %lu\n", j);
                }
            }
        }
    }
    // printf("j is: %lu\n", j);
    if (j != j_start)
    {
        i--;
        j--;
        size_t len = j - j_start + 2;
        new_args[new_args_index] = (char *)calloc(len, sizeof(char));
        strncpy(new_args[new_args_index], &(args[i][j_start]), len - 1);
    }
    for (size_t i = 0; i < *size; i++)
        free(args[i]);
    free(args);
    *size = args_count;
    return new_args;
}

static int exec_for_loop(struct varmap *vars, struct ast_node *node)
{
    struct for_loop_node *for_loop = node->body.for_loop;
    size_t nb = for_loop->args->size;
    char **args = for_loop->args->data;
    args = expand_args(vars, args, &nb, 0);
    char *ifs = varmap_get(vars, "IFS");
    size_t args_count;
    if (ifs) // TODO: No field splitting if in double quotes?
    {
        args = remove_empty_strings(args, &nb);
        args_count = count_args_IFS(args, &nb, ifs);
        args = handle_IFS(args, &nb, ifs, args_count);
    }
    for_loop->args->size = nb;
    for_loop->args->data = args;

    int status;
    for (size_t i = 0; i < nb; i++)
    {
        if (break_flag)
        {
            break_flag = 0;
            break;
        }
        if (continue_flag)
        {
            continue_flag = 0;
            continue;
        }
        varmap_insert(vars, for_loop->varname, args[i]);
        status = exec_ast(vars, for_loop->body);
        if (status != 0)
            break;
    }
    return status;
}

static int exec_neg(struct varmap *vars, struct ast_node *node)
{
    struct neg_node *neg = node->body.negation;
    int res = exec_ast(vars, neg->elem);
    return (res == 0) ? 1 : 0;
}

static int exec_var(struct varmap *vars, struct ast_node *node)
{
    struct var_node *var = node->body.variable;
    if (!var || !var->assignment)
        return 2;

    int dollar = 0;
    while ((dollar = contains_subshell(var->assignment)))
    {
        char *subshell = NULL;
        char *toreplace = NULL;

        parse_subshell(var->assignment, &subshell, &toreplace, dollar);

        char *value = exec_var_subshell(vars, subshell);
        if (!value)
        {
            value = calloc(1, sizeof(char));
        }

        char *str = var->assignment;
        var->assignment = replace_var(var->assignment, toreplace, value);

        free(str);
        free(value);
        free(subshell);
        free(toreplace);
    }

    varmap_insert_ass(vars, var->assignment);
    return 0;
}

static int exec_func(struct varmap *vars, struct ast_node *node)
{
    vars = vars + 0;
    struct func_node *func = node->body.function;
    if (!func || !func->body || !func->name)
        return 2;
    if (func->redir)
    {
        struct ast_node *redir = func->redir;
        while (redir->body.redir->elem)
            redir = redir->body.redir->elem;
        redir->body.redir->elem = func->body;
        funcmap_insert(func->name, redir);
        redir->body.redir->elem = NULL;
    }
    else
        funcmap_insert(func->name, func->body);
    return 0;
}

static char *extract_varname(char *var)
{
    size_t i = 0;
    while (var[i] != '$' && var[i] != '\0')
        i += 1;
    if (var[i] == '\0')
        return strdup(var);

    i += 1;
    size_t curly = var[i] == '{';
    i += curly;
    size_t start = i;
    while (var[i] != '\0')
        i += 1;

    return strndup(var + start, i - curly - start);
}

static int is_regex_char(char c)
{
    return c == '.' || c == '*' || c == '+' || c == '?' || c == '|' || c == '['
        || c == ']';
}

static int is_regex(const char *str)
{
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (is_regex_char(str[i]))
            return 1;
    }
    return 0;
}

static int is_anything_regex(char c, char reg)
{
    return (reg == c || (reg == '!' && !(isalpha(c) || isdigit(c))));
}

enum char_class
{
    ALNUM,
    ALPHA,
    BLANK,
    CNTRL,
    DIGIT,
    GRAPH,
    LOWER,
    PRINT,
    PUNCT,
    SPACE,
    UPPER,
    XDIGIT,
    NOPE
};

struct class
{
    char *str;
    enum char_class type;
};

static enum char_class get_class_type(char *regex)
{
    struct class class[] = { { ":alnum:", ALNUM }, { ":alpha:", ALPHA },
                             { ":blank:", BLANK }, { ":cntrl:", CNTRL },
                             { ":digit:", DIGIT }, { ":graph:", GRAPH },
                             { ":lower:", LOWER }, { ":print:", PRINT },
                             { ":punct:", PUNCT }, { ":space:", SPACE },
                             { ":upper:", UPPER }, { ":xdigit:", XDIGIT } };
    for (size_t i = 0; i < 12; i++)
    {
        if (strcmp(class[i].str, regex) == 0)
            return class[i].type;
    }
    return NOPE;
}

static char *extract_class_name(const char *regex, size_t j)
{
    size_t start = j;
    j += 1;
    while (regex[j] != ':' && regex[j] != '\0')
        j += 1;
    return strndup(regex + start, j - start + 1);
}

static int isnone(int c)
{
    return c - c;
}

typedef int (*class_handler)(int c);

static int handle_posix_classes(const char *regex, size_t j, int c)
{
    class_handler tb[] = { isalnum, isalpha,  isblank, iscntrl, isdigit,
                           isgraph, islower,  isprint, ispunct, isspace,
                           isupper, isxdigit, isnone };

    char *reg = extract_class_name(regex, j);
    int res = tb[get_class_type(reg)](c);
    free(reg);
    return res;
}

static int match_regex(const char *str, const char *regex)
{
    if (!is_regex(regex))
        return 0;
    for (size_t i = 0; i < strlen(str); i++)
    {
        for (size_t j = 0; j < strlen(regex); j++)
        {
            switch (regex[j])
            {
            case '*': {
                if (is_anything_regex(str[i], regex[j + 1]))
                    return 1;
            }
            break;
            case '[':
                j++;
                while (regex[j] != ']')
                {
                    if (regex[j] == str[i])
                    {
                        return 1;
                    }
                    if (regex[j] == '-' && str[i] >= regex[j - 1]
                        && str[i] <= regex[j + 1])
                        return 1;
                    if (regex[j] == ':'
                        && handle_posix_classes(regex, j, str[i]))
                        return 1;
                    j++;
                }
                break;
            case '.':
                return 1;
            case ':':
                if (handle_posix_classes(regex, j, str[i]))
                    return 1;
                break;
            default:
                if (regex[j] == str[i])
                {
                    return 1;
                }
                return 0;
            }
        }
    }
    return 0;
}

static int exec_case_cmd(struct varmap *vars, struct ast_node *node)
{
    if (!node)
        return 2;
    struct case_node *case_cmd = node->body.case_cmd;
    if (!case_cmd || !case_cmd->var || !case_cmd->clause)
        return 2;

    char *name = expand_str(strdup(case_cmd->var), 1);
    char *varname = extract_varname(name);
    free(name);
    char *tomatch = varmap_get(vars, varname);
    free(varname);

    if (!tomatch)
        return 0;

    for (struct ast_node *p = case_cmd->clause; p; p = p->next)
    {
        struct case_item_node *tp = p->body.case_item;
        if (!tp)
            continue;
        int match = 0;
        for (size_t i = 0; i < tp->to_match->size; i++)
        {
            char *arg = expand_str(strdup(tp->to_match->data[i]), 1);
            int reg = 0;
            if (strcmp(tomatch, arg) == 0 || (reg = match_regex(tomatch, arg)))
            {
                free(arg);
                match = 1;
                break;
            }
            free(arg);
        }
        if (!match)
            continue;
        return exec_ast(vars, tp->body);
    }
    return 0;
}

/* EXEC AST JUMP TABLE */
typedef int (*exec_handler)(struct varmap *vars, struct ast_node *node);

static exec_handler exec_jump_table[] = {
    exec_simple_cmd, exec_if_cmd,     exec_cmp_list,   exec_redir,    exec_pipe,
    exec_and_or,     exec_while_loop, exec_until_loop, exec_for_loop, exec_neg,
    exec_var,        exec_func,       exec_case_cmd
};

int exec_ast(struct varmap *vars, struct ast_node *node)
{
    if (!node)
        return 0;

    int error_code = exec_jump_table[node->type](vars, node);

    char *error_code_str = my_itoa(error_code);
    varmap_insert(vars, "?", error_code_str);
    free(error_code_str);

    return error_code;
}
