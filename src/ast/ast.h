#ifndef AST_H
#define AST_H

#include <stddef.h>

#include "../vector/vector.h"

/* SIMPLE COMMAND */
struct simple_cmd_node
{
    struct ast_node *vars;
    char *cmd;
    struct vector *args;
};

/* IF COMMAND */
struct if_cmd_node
{
    struct ast_node *cond;
    struct ast_node *cond_true;
    struct ast_node *cond_false;
};

enum cmp_type
{
    SUBSHELL,
    CMD_BLOCK,
    LIST
};

/* COMPOUND LIST */
struct cmp_list_node
{
    enum cmp_type type;
    struct ast_node *elem;
};

/* REDIRECTION */
enum redir_type
{
    MORE, // >
    LESS, // <
    DMORE, // >>
    MOREAND, // >&
    LESSAND, // <&
    MOREOR, // >|
    LESSMORE // <>
};

struct redir_file
{
    int fd;
    char *path;
};

struct redir_node
{
    struct redir_file *src;
    struct redir_file *dest;
    enum redir_type type;
    struct ast_node *elem;
};

/* PIPELINE */
struct pipeline_node
{
    struct ast_node *elem;
};

enum pipe_type
{
    AND,
    OR
};

struct and_or_node
{
    enum pipe_type type;
    struct ast_node *left;
    struct ast_node *right;
};

/* NEGATION */
struct neg_node
{
    struct ast_node *elem;
};

/* LOOPS : WHILE, UNTIL */
enum loop_type
{
    WHILE,
    UNTIL
};

struct loop_node
{
    enum loop_type type;
    struct ast_node *cond;
    struct ast_node *body;
};

/* FOR LOOP */
struct for_loop_node
{
    char *varname;
    struct vector *args;
    struct ast_node *body;
};

/* VARIABLE */
struct var_node
{
    char *assignment;
};

/* FUNCTION */
struct func_node
{
    char *name;
    struct ast_node *body;
    struct ast_node *redir;
};

/* CASE COMMAND */
struct case_item_node
{
    struct vector *to_match;
    struct ast_node *body;
};

struct case_node
{
    char *var;
    struct ast_node *clause;
};

/* ------------------------------- AST BODY -------------------------------- */
union ast_body
{
    struct simple_cmd_node *simple_cmd;
    struct if_cmd_node *if_cmd;
    struct cmp_list_node *cmp_list;
    struct redir_node *redir;
    struct pipeline_node *pipe;
    struct and_or_node *and_or;
    struct loop_node *while_loop;
    struct loop_node *until_loop;
    struct for_loop_node *for_loop;
    struct neg_node *negation;
    struct var_node *variable;
    struct func_node *function;
    struct case_node *case_cmd;
    struct case_item_node *case_item;
};

/* ------------------------------- AST TYPE -------------------------------- */

enum ast_type
{
    SIMPLE_CMD,
    IF_CMD,
    CMP_LIST,
    REDIR,
    PIPE,
    AND_OR,
    WHILE_LOOP,
    UNTIL_LOOP,
    FOR_LOOP,
    NEG,
    VAR,
    FUNC,
    CASE_CMD,
    CASE_ITEM,
};

/* ------------------------------- AST NODE -------------------------------- */
struct ast_node
{
    enum ast_type type;
    union ast_body body;
    struct ast_node *next;
};

/* -------------------------- FUNCTIONS TO EXPORT -------------------------- */

/* RETURNS an allocated ast_node with type type, fields are NOT set AND NULL */
struct ast_node *ast_node_init(enum ast_type type);

/* PRETTY PRINT AST NODE */
void ast_pretty_print(struct ast_node *node);

/* DESTROY AST NODE */
void ast_node_destroy(struct ast_node *node);

/* DUPLICATE AST */
struct ast_node *ast_dup(struct ast_node *node);

/* REDIR FILE INIT */
struct redir_file *redir_file_init(void);

/* REDIR FILE DESTROY */
void redir_file_destroy(struct redir_file *r);

#endif /* ! AST_H */
