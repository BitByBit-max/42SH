#ifndef EXEC_H
#define EXEC_H

#include "../ast/ast.h"
#include "../varmap/varmap.h"

int exec_ast(struct varmap *varmap, struct ast_node *node);

#endif /* ! EXEC_H */
