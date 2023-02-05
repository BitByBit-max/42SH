#ifndef VARMAP_H
#define VARMAP_H

#include <stddef.h>

struct var
{
    char *name;
    char *value;
};

struct pair_list
{
    struct var *var;
    struct pair_list *next;
};

struct varmap
{
    struct pair_list **data;
    size_t size;
};

/* VARMAP INIT */
struct varmap *varmap_init(size_t size);

/* VARMAP DESTROY */
void varmap_destroy(struct varmap *vars);

/* VARMAP INSERT ASSIGNMENT */
int varmap_insert_ass(struct varmap *vars, char *assingment);

/* VARMAP DESTROY */
int varmap_remove(struct varmap *vars, char *varname);

/* VARMAP INSERT */
int varmap_insert(struct varmap *vars, char *name, char *value);

/* VARMAP GET */
char *varmap_get(struct varmap *vars, const char *varname);

/* VARMAP PRINT */
void varmap_print(struct varmap *vars);

/* =========== EXPANSION =========== */

char *replace_var(char *str, char *varname, char *value);

/* IS EXPANDABLE */
int is_expandable(char *str);

/* EXPAND VAR */
char *expand_var(struct varmap *vars, char *str);

#endif /* ! VARMAP_H */
