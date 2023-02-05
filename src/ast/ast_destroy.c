#include <stdlib.h>

#include "../vector/vector.h"
#include "ast.h"

/* -------------------------------- DESTROY -------------------------------- */

/* SIMPLE COMMAND DESTROY */
static void simple_cmd_destroy(struct ast_node *node)
{
    if (!node || !node->body.simple_cmd)
        return;

    struct simple_cmd_node *simp = node->body.simple_cmd;

    if (simp->cmd)
        free(simp->cmd);

    if (simp->args)
        vector_destroy(simp->args);

    free(simp);
}

/* IF COMMAND DESTROY */
static void if_cmd_destroy(struct ast_node *node)
{
    if (!node || !node->body.if_cmd)
        return;

    struct if_cmd_node *if_cmd = node->body.if_cmd;
    ast_node_destroy(if_cmd->cond);
    ast_node_destroy(if_cmd->cond_true);
    ast_node_destroy(if_cmd->cond_false);
    free(if_cmd);
}

/* COMPOUND LIST DESTROY */
static void cmp_list_destroy(struct ast_node *node)
{
    if (!node || !node->body.cmp_list)
        return;

    struct cmp_list_node *cmp = node->body.cmp_list;
    struct ast_node *p = cmp->elem;
    while (p)
    {
        struct ast_node *temp = p->next;
        ast_node_destroy(p);
        p = temp;
    }
    free(cmp);
}

/* REDIR DESTROY */
void redir_file_destroy(struct redir_file *r)
{
    if (!r)
        return;
    free(r->path);
    free(r);
}

static void redir_destroy(struct ast_node *node)
{
    if (!node || !node->body.redir)
        return;

    struct redir_node *redir = node->body.redir;
    redir_file_destroy(redir->src);
    redir_file_destroy(redir->dest);

    ast_node_destroy(redir->elem);
    free(redir);
}

/* PIPELINE DESTROY */
static void pipe_destroy(struct ast_node *node)
{
    if (!node || !node->body.pipe)
        return;

    struct pipeline_node *pipe = node->body.pipe;
    struct ast_node *p = pipe->elem;
    while (p)
    {
        struct ast_node *temp = p->next;
        ast_node_destroy(p);
        p = temp;
    }
    free(pipe);
}

/* AND OR DESTROY */
static void and_or_destroy(struct ast_node *node)
{
    if (!node || !node->body.and_or)
        return;
    struct and_or_node *and_or = node->body.and_or;
    ast_node_destroy(and_or->left);
    ast_node_destroy(and_or->right);
    free(and_or);
}

/* WHILE LOOP DESTROY */
static void while_loop_destroy(struct ast_node *node)
{
    if (!node || !node->body.while_loop)
        return;
    struct loop_node *while_l = node->body.while_loop;
    ast_node_destroy(while_l->cond);
    ast_node_destroy(while_l->body);
    free(while_l);
}

/* UNTIL LOOP DESTROY */
static void until_loop_destroy(struct ast_node *node)
{
    if (!node || !node->body.until_loop)
        return;
    struct loop_node *until_l = node->body.until_loop;
    ast_node_destroy(until_l->cond);
    ast_node_destroy(until_l->body);
    free(until_l);
}

/* FOR DESTROY */
static void for_loop_destroy(struct ast_node *node)
{
    if (!node || !node->body.for_loop)
        return;
    struct for_loop_node *for_loop = node->body.for_loop;
    if (for_loop->varname)
        free(for_loop->varname);
    if (for_loop->args)
        vector_destroy(for_loop->args);
    ast_node_destroy(for_loop->body);
    free(for_loop);
}

/* NEG DESTROY */
static void neg_destroy(struct ast_node *node)
{
    if (!node || !node->body.negation)
        return;
    struct neg_node *neg = node->body.negation;
    ast_node_destroy(neg->elem);
    free(neg);
}

/* VAR DESTROY */
static void var_destroy(struct ast_node *node)
{
    if (!node || !node->body.variable)
        return;
    struct var_node *var = node->body.variable;
    if (var->assignment)
        free(var->assignment);
    free(var);
}

static void func_destroy(struct ast_node *node)
{
    if (!node || !node->body.function)
        return;
    struct func_node *func = node->body.function;
    free(func->name);
    ast_node_destroy(func->body);
    ast_node_destroy(func->redir);
    free(func);
}

static void case_cmd_destroy(struct ast_node *node)
{
    if (!node || !node->body.case_cmd)
        return;
    struct case_node *case_cmd = node->body.case_cmd;
    free(case_cmd->var);
    struct ast_node *p = case_cmd->clause;

    while (p)
    {
        struct ast_node *temp = p->next;
        ast_node_destroy(p);
        p = temp;
    }
    free(case_cmd);
}

static void case_item_destroy(struct ast_node *node)
{
    if (!node || !node->body.case_item)
        return;
    struct case_item_node *case_item = node->body.case_item;
    vector_destroy(case_item->to_match);
    ast_node_destroy(case_item->body);
    free(case_item);
}

/* AST NODE DESTROY JUMP TABLE */
typedef void (*ast_destroy_handler)(struct ast_node *node);

static const ast_destroy_handler destroy_jump_table[14] = {
    simple_cmd_destroy, if_cmd_destroy,     cmp_list_destroy,
    redir_destroy,      pipe_destroy,       and_or_destroy,
    while_loop_destroy, until_loop_destroy, for_loop_destroy,
    neg_destroy,        var_destroy,        func_destroy,
    case_cmd_destroy,   case_item_destroy
};

/* AST NODE DESTROY */
void ast_node_destroy(struct ast_node *node)
{
    if (!node)
        return;

    destroy_jump_table[node->type](node);
    free(node);
}
