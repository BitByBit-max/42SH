#ifndef FUNCMAP_H
#define FUNCMAP_H

#include <stddef.h>

#include "../ast/ast.h"

struct func
{
    char *name;
    struct ast_node *body;
};

struct pair_list_func
{
    struct func *func;
    struct pair_list_func *next;
};

struct funcmap
{
    struct pair_list_func **data;
    size_t size;
};

/* funcmap INIT */
void funcmap_init(size_t size);

/* funcmap DESTROY */
void funcmap_destroy(void);

/* funcmap INSERT */
int funcmap_insert(char *name, struct ast_node *body);

/* FUNCMAP REMOVE */
int funcmap_remove(char *funcname);

/* funcmap GET */
struct func *funcmap_get(const char *funcname);

/* funcmap PRINT */
void funcmap_print(void);

#endif /* ! FUNCMAP_H */
