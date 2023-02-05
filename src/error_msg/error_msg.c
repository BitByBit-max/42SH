#include "error_msg.h"

#include <stdio.h>

#include "../aliasmap/aliasmap.h"
#include "../ast/ast.h"
#include "../funcmap/funcmap.h"
#include "../io/io.h"
#include "../log/log.h"
#include "../varmap/varmap.h"

int error_io(struct varmap *vars)
{
    fprintf(stderr, "Usage: 42sh [OPTIONS] [SCRIPT] [ARGUMENTS ...]\n");
    log_text("Error IO.");
    close_file();
    varmap_destroy(vars);
    funcmap_destroy();
    aliasmap_destroy();
    return 2;
}

int empty_str_io(struct varmap *vars)
{
    log_text("Error IO.");
    close_file();
    varmap_destroy(vars);
    funcmap_destroy();
    aliasmap_destroy();
    return 0;
}

int error_parse(struct ast_node *tree, struct varmap *vars)
{
    fprintf(stderr, "Error while parsing\n");
    log_text("Error build");
    ast_node_destroy(tree);
    varmap_destroy(vars);
    funcmap_destroy();
    aliasmap_destroy();
    io_end();
    close_file();
    return 2;
}

int error_exec(struct ast_node *tree, struct varmap *vars)
{
    fprintf(stderr, "Error while executing\n");
    log_text("Error exec");
    ast_node_destroy(tree);
    varmap_destroy(vars);
    funcmap_destroy();
    aliasmap_destroy();
    io_end();
    close_file();
    return 2;
}
