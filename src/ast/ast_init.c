#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"

/* --------------------------------- INIT ---------------------------------- */

/* SIMPLE COMMAND INIT */
static struct ast_node *simple_cmd_init(struct ast_node *node)
{
    struct simple_cmd_node *new = calloc(1, sizeof(struct simple_cmd_node));
    if (!new)
        return NULL;
    new->cmd = NULL;
    new->args = NULL;
    node->body.simple_cmd = new;
    return node;
}

/* IF COMMAND INIT */
static struct ast_node *if_cmd_init(struct ast_node *node)
{
    struct if_cmd_node *new = calloc(1, sizeof(struct if_cmd_node));
    if (!new)
        return NULL;
    new->cond = NULL;
    new->cond_true = NULL;
    new->cond_false = NULL;
    node->body.if_cmd = new;
    return node;
}

/* COMPOUND LIST INIT */
static struct ast_node *cmp_list_init(struct ast_node *node)
{
    struct cmp_list_node *new = calloc(1, sizeof(struct cmp_list_node));
    if (!new)
        return NULL;
    new->elem = NULL;
    node->body.cmp_list = new;
    return node;
}

/* REDIR INIT */
static struct ast_node *redir_init(struct ast_node *node)
{
    struct redir_node *new = calloc(1, sizeof(struct redir_node));
    if (!new)
        return NULL;
    new->src = NULL;
    new->dest = NULL;
    new->elem = NULL;
    node->body.redir = new;
    return node;
}

/* PIPELINE INIT */
static struct ast_node *pipe_init(struct ast_node *node)
{
    struct pipeline_node *new = calloc(1, sizeof(struct pipeline_node));
    if (!new)
        return NULL;
    new->elem = NULL;
    node->body.pipe = new;
    return node;
}

/* AND OR INIT */
static struct ast_node *and_or_init(struct ast_node *node)
{
    struct and_or_node *new = calloc(1, sizeof(struct and_or_node));
    if (!new)
        return NULL;
    new->left = NULL;
    new->right = NULL;
    node->body.and_or = new;
    return node;
}

/* WHILE LOOP INIT */
static struct ast_node *while_loop_init(struct ast_node *node)
{
    struct loop_node *new = calloc(1, sizeof(struct loop_node));
    if (!new)
        return NULL;
    new->type = WHILE;
    new->cond = NULL;
    new->body = NULL;
    node->body.while_loop = new;
    return node;
}

/* UNTIL LOOP INIT */
static struct ast_node *until_loop_init(struct ast_node *node)
{
    struct loop_node *new = calloc(1, sizeof(struct loop_node));
    if (!new)
        return NULL;
    new->type = UNTIL;
    new->cond = NULL;
    new->body = NULL;
    node->body.until_loop = new;
    return node;
}

/* FOR LOOP INIT */
static struct ast_node *for_loop_init(struct ast_node *node)
{
    struct for_loop_node *new = calloc(1, sizeof(struct for_loop_node));
    if (!new)
        return NULL;
    new->varname = NULL;
    new->args = NULL;
    new->body = NULL;
    node->body.for_loop = new;
    return node;
}

/* NEG NODE INIT */
static struct ast_node *neg_init(struct ast_node *node)
{
    struct neg_node *new = calloc(1, sizeof(struct neg_node));
    if (!new)
        return NULL;
    new->elem = NULL;
    node->body.negation = new;
    return node;
}

/* REDIR FILE INIT */
struct redir_file *redir_file_init(void)
{
    struct redir_file *new = calloc(1, sizeof(struct redir_file));
    if (!new)
        return NULL;
    new->fd = -1;
    new->path = NULL;
    return new;
}

static struct ast_node *var_init(struct ast_node *node)
{
    struct var_node *new = calloc(1, sizeof(struct var_node));
    if (!new)
        return NULL;
    new->assignment = NULL;
    node->body.variable = new;
    return node;
}

static struct ast_node *func_init(struct ast_node *node)
{
    struct func_node *new = calloc(1, sizeof(struct func_node));
    if (!new)
        return NULL;
    new->name = NULL;
    new->redir = NULL;
    new->body = NULL;
    node->body.function = new;
    return node;
}

static struct ast_node *case_cmd_init(struct ast_node *node)
{
    struct case_node *new = calloc(1, sizeof(struct case_node));
    if (!new)
        return NULL;
    new->var = NULL;
    new->clause = NULL;
    node->body.case_cmd = new;
    return node;
}

static struct ast_node *case_item_init(struct ast_node *node)
{
    struct case_item_node *new = calloc(1, sizeof(struct case_item_node));
    if (!new)
        return NULL;
    new->to_match = NULL;
    new->body = NULL;
    node->body.case_item = new;
    return node;
}

/* AST NODE INIT JUMP TABLE */
typedef struct ast_node *(*ast_init_handler)(struct ast_node *node);

static const ast_init_handler init_jump_table[14] = {
    simple_cmd_init, if_cmd_init,     cmp_list_init,   redir_init,    pipe_init,
    and_or_init,     while_loop_init, until_loop_init, for_loop_init, neg_init,
    var_init,        func_init,       case_cmd_init,   case_item_init
};

/* AST NODE INIT */
struct ast_node *ast_node_init(enum ast_type type)
{
    struct ast_node *new = calloc(1, sizeof(struct ast_node));
    if (!new)
        return NULL;
    new->type = type;

    return init_jump_table[type](new);
}
