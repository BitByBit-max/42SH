#ifndef ERROR_MSG_H
#define ERROR_MSG_H

#include "../ast/ast.h"
#include "../varmap/varmap.h"

int empty_str_io(struct varmap *vars);

int error_io(struct varmap *vars);

int error_parse(struct ast_node *tree, struct varmap *vars);

int error_exec(struct ast_node *tree, struct varmap *vars);

#endif /* ERROR_MSG ! */
