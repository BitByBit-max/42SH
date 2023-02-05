#include "../ast/ast.h"
#include "../error_msg/error_msg.h"
#include "../exec/exec.h"
#include "../io/io.h"
#include "../parser/parser.h"
#include "../varmap/special_vars.h"
#include "../varmap/varmap.h"

int run(struct varmap *vars, int pretty_print)
{
    int exitcode = 0;
    struct ast_node *tree = NULL;
    do
    {
        if (io_generate() == EOF)
        {
            break;
        }
        tree = build(&exitcode);
        fflush(io_get_fd());
        if (exitcode == 2)
        {
            fprintf(stderr, "error parsing\n");
            char *str = my_itoa(2);
            varmap_insert(vars, "?", str);
            free(str);
            ast_node_destroy(tree);
            if (io_generate() == EOF)
                break;
            continue;
        }
        // return error_parse(tree, vars);
        if (!tree)
        {
            if (exitcode == 3)
            {
                exitcode = 0;
                continue;
            }
            break;
        }
        if (pretty_print)
            ast_pretty_print(tree);
        else
            exitcode = exec_ast(vars, tree);

        ast_node_destroy(tree);
    } while (1);
    return exitcode;
}
