#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
/* --------------------------------- DUP ----------------------------------- */
static struct vector *vector_dup(struct vector *tocopy)
{
    struct vector *res = vector_init(tocopy->size + 1);

    for (size_t i = 0; i < tocopy->size; i++)
    {
        res = vector_append(res, strdup(tocopy->data[i]));
    }
    return res;
}

static struct ast_node *simple_cmd_dup(struct ast_node *tocopy,
                                       struct ast_node *res)
{
    if (!tocopy)
        return res;

    res->body.simple_cmd->cmd = strdup(tocopy->body.simple_cmd->cmd);
    res->body.simple_cmd->vars = ast_dup(tocopy->body.simple_cmd->vars);
    res->body.simple_cmd->args = vector_dup(tocopy->body.simple_cmd->args);

    return res;
}

static struct ast_node *if_cmd_dup(struct ast_node *tocopy,
                                   struct ast_node *res)
{
    if (!tocopy)
        return res;
    res->body.if_cmd->cond = ast_dup(tocopy->body.if_cmd->cond);
    res->body.if_cmd->cond_true = ast_dup(tocopy->body.if_cmd->cond_true);
    res->body.if_cmd->cond_false = ast_dup(tocopy->body.if_cmd->cond_false);
    return res;
}

static struct ast_node *cmp_list_dup(struct ast_node *tocopy,
                                     struct ast_node *res)
{
    if (!tocopy)
        return res;

    res->body.cmp_list->type = tocopy->body.cmp_list->type;
    struct ast_node *p; // = res->elem;
    struct ast_node *head = NULL;
    struct ast_node *prev = NULL;
    for (struct ast_node *node = tocopy->body.cmp_list->elem; node;
         node = node->next)
    {
        p = ast_dup(node);
        if (prev)
            prev->next = p;
        if (!head)
            head = p;
        prev = p;
        p = p->next;
    }
    res->body.cmp_list->elem = head;
    return res;
}

static struct redir_file *redir_file_dup(struct redir_file *tocopy)
{
    if (!tocopy)
        return NULL;
    struct redir_file *res = redir_file_init();
    res->fd = tocopy->fd;
    res->path = tocopy->path ? strdup(tocopy->path) : NULL;
    return res;
}

static struct ast_node *redir_dup(struct ast_node *tocopy, struct ast_node *res)
{
    if (!tocopy)
        return res;

    res->body.redir->type = tocopy->body.redir->type;
    res->body.redir->src = redir_file_dup(tocopy->body.redir->src);
    res->body.redir->dest = redir_file_dup(tocopy->body.redir->dest);
    res->body.redir->elem = ast_dup(tocopy->body.redir->elem);

    return res;
}

static struct ast_node *pipe_dup(struct ast_node *tocopy, struct ast_node *res)
{
    if (!tocopy)
        return res;

    struct ast_node *p; // = res->elem;
    struct ast_node *head = NULL;
    struct ast_node *prev = NULL;
    for (struct ast_node *node = tocopy->body.pipe->elem; node;
         node = node->next)
    {
        p = ast_dup(node);
        if (prev)
            prev->next = p;
        if (!head)
            head = p;
        prev = p;
        p = p->next;
    }
    res->body.pipe->elem = head;
    return res;
}

static struct ast_node *and_or_dup(struct ast_node *tocopy,
                                   struct ast_node *res)
{
    if (!tocopy)
        return res;
    res->body.and_or->type = tocopy->body.and_or->type;
    res->body.and_or->left = ast_dup(tocopy->body.and_or->left);
    res->body.and_or->right = ast_dup(tocopy->body.and_or->right);
    return res;
}

static struct ast_node *while_loop_dup(struct ast_node *tocopy,
                                       struct ast_node *res)
{
    if (!tocopy)
        return res;
    res->body.while_loop->type = tocopy->body.while_loop->type;
    res->body.while_loop->cond = ast_dup(tocopy->body.while_loop->cond);
    res->body.while_loop->body = ast_dup(tocopy->body.while_loop->body);
    return res;
}

static struct ast_node *until_loop_dup(struct ast_node *tocopy,
                                       struct ast_node *res)
{
    if (!tocopy)
        return res;
    res->body.until_loop->type = tocopy->body.until_loop->type;
    res->body.until_loop->cond = ast_dup(tocopy->body.until_loop->cond);
    res->body.until_loop->body = ast_dup(tocopy->body.until_loop->body);
    return res;
}

static struct ast_node *for_loop_dup(struct ast_node *tocopy,
                                     struct ast_node *res)
{
    if (!tocopy)
        return res;
    res->body.for_loop->varname = strdup(tocopy->body.for_loop->varname);
    res->body.for_loop->args = vector_dup(tocopy->body.for_loop->args);
    res->body.for_loop->body = ast_dup(tocopy->body.for_loop->body);
    return res;
}

static struct ast_node *neg_dup(struct ast_node *tocopy, struct ast_node *res)
{
    if (!tocopy)
        return res;
    res->body.negation->elem = ast_dup(tocopy->body.negation->elem);
    return res;
}

static struct ast_node *var_dup(struct ast_node *tocopy, struct ast_node *res)
{
    if (!tocopy)
        return res;
    res->body.variable->assignment = strdup(tocopy->body.variable->assignment);
    return res;
}

static struct ast_node *func_dup(struct ast_node *tocopy, struct ast_node *res)
{
    if (!tocopy)
        return res;
    res->body.function->name = strdup(tocopy->body.function->name);
    res->body.function->body = ast_dup(tocopy->body.function->body);
    return res;
}

static struct ast_node *case_cmd_dup(struct ast_node *tocopy,
                                     struct ast_node *res)
{
    if (!tocopy)
        return res;
    res->body.case_cmd->var = strdup(tocopy->body.case_cmd->var);

    struct ast_node *p; // = res->elem;
    struct ast_node *head = NULL;
    struct ast_node *prev = NULL;
    for (struct ast_node *node = tocopy->body.case_cmd->clause; node;
         node = node->next)
    {
        p = ast_dup(node);
        if (prev)
            prev->next = p;
        if (!head)
            head = p;
        prev = p;
        p = p->next;
    }
    res->body.case_cmd->clause = head;
    return res;
}

static struct ast_node *case_item_dup(struct ast_node *tocopy,
                                      struct ast_node *res)
{
    if (!tocopy)
        return res;

    res->body.case_item->to_match =
        vector_dup(tocopy->body.case_item->to_match);
    res->body.case_item->body = ast_dup(tocopy->body.case_item->body);

    return res;
}

typedef struct ast_node *(*ast_dup_handler)(struct ast_node *tocopy,
                                            struct ast_node *res);

static const ast_dup_handler dup_jump_table[14] = {
    simple_cmd_dup, if_cmd_dup,     cmp_list_dup,   redir_dup,    pipe_dup,
    and_or_dup,     while_loop_dup, until_loop_dup, for_loop_dup, neg_dup,
    var_dup,        func_dup,       case_cmd_dup,   case_item_dup
};

struct ast_node *ast_dup(struct ast_node *node)
{
    if (!node)
        return NULL;
    struct ast_node *res = ast_node_init(node->type);

    return dup_jump_table[res->type](node, res);
}
