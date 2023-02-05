#include <stddef.h>
#include <stdio.h>

#include "ast.h"

/* ----------------------------- PRETTY PRINT ------------------------------ */

/* SIMPLE COMMAND PRINT */
static void simple_cmd_print(struct ast_node *toprint)
{
    struct simple_cmd_node *node = toprint->body.simple_cmd;
    if (!node)
    {
        printf("NULL");
        return;
    }
    printf("command \"");
    printf("%s", node->cmd);
    printf("\"");
    if (node->args)
    {
        for (size_t i = 0; i < node->args->size; i++)
        {
            printf(" \"");
            printf("%s", node->args->data[i]);
            printf("\"");
        }
    }
}

/* IF COMMAND PRINT */
static void if_cmd_print(struct ast_node *toprint)
{
    struct if_cmd_node *node = toprint->body.if_cmd;
    if (!node)
    {
        printf("NULL");
        return;
    }
    printf("if { ");
    ast_pretty_print(node->cond);
    printf(" }; then { ");
    ast_pretty_print(node->cond_true);
    if (node->cond_false)
    {
        printf(" }; else { ");
        ast_pretty_print(node->cond_false);
    }
    printf(" }");
}

static void print_cmp_symbol(enum cmp_type type, int start)
{
    switch (type)
    {
    case SUBSHELL:
        if (start == 0)
            printf("( ");
        else
            printf(" )");
        break;
    case CMD_BLOCK:
        if (start == 0)
            printf("{ ");
        else
            printf(" }");
        break;
    default:
        break;
    }
}

/* COMPOUND LIST PRINT */
static void cmp_list_print(struct ast_node *toprint)
{
    struct cmp_list_node *node = toprint->body.cmp_list;
    if (!node)
    {
        printf("NULL");
        return;
    }
    print_cmp_symbol(node->type, 0);
    struct ast_node *p = node->elem;
    while (p)
    {
        ast_pretty_print(p);
        printf(";");
        p = p->next;
    }
    print_cmp_symbol(node->type, 1);
}

/* REDIR PRINT */
static void print_redir_type(enum redir_type type)
{
    switch (type)
    {
    case MORE: // >
        printf(">");
        break;
    case LESS: // <
        printf("<");
        break;
    case DMORE: // >>
        printf(">>");
        break;
    case MOREAND: // >&
        printf(">&");
        break;
    case LESSAND: // <&
        printf("<&");
        break;
    case MOREOR: // >|
        printf(">|");
        break;
    case LESSMORE: // <>
        printf("<>");
        break;
    }
}

static void redir_print(struct ast_node *toprint)
{
    struct redir_node *node = toprint->body.redir;
    if (!node)
    {
        printf("NULL");
        return;
    }
    printf("redir ");
    if (node->src)
    {
        if (node->src->fd != -1)
            printf("\"%d\"", node->src->fd);
        else
            printf("\"%s\"", node->src->path);
    }
    print_redir_type(node->type);
    if (node->dest)
    {
        if (node->dest->fd != -1)
            printf("\"%d\"", node->dest->fd);
        else
            printf("\"%s\"", node->dest->path);
    }
    printf(" ");
    ast_pretty_print(node->elem);
}

/* PIPELINE PRINT */
static void pipe_print(struct ast_node *toprint)
{
    struct pipeline_node *node = toprint->body.pipe;
    if (!node)
    {
        printf("NULL");
        return;
    }
    struct ast_node *p = node->elem;
    while (p)
    {
        ast_pretty_print(p);
        p = p->next;
        if (p)
            printf(" | ");
    }
}

static void print_pipe_symbol(enum pipe_type type)
{
    switch (type)
    {
    case AND:
        printf(" && ");
        break;
    case OR:
        printf(" || ");
        break;
    }
}

static void and_or_print(struct ast_node *toprint)
{
    struct and_or_node *node = toprint->body.and_or;
    if (!node)
    {
        printf("NULL");
        return;
    }
    ast_pretty_print(node->left);
    print_pipe_symbol(node->type);
    ast_pretty_print(node->right);
}

/* WHILE LOOP PRINT */
static void while_loop_print(struct ast_node *toprint)
{
    struct loop_node *node = toprint->body.while_loop;
    if (!node)
    {
        printf("NULL");
        return;
    }
    printf("while { ");

    ast_pretty_print(node->cond);
    printf(" }; \ndo\n { ");
    ast_pretty_print(node->body);
    printf(" }\ndone");
}

/* UNTIL LOOP PRINT */
static void until_loop_print(struct ast_node *toprint)
{
    struct loop_node *node = toprint->body.until_loop;
    if (!node)
    {
        printf("NULL");
        return;
    }
    printf("until { ");

    ast_pretty_print(node->cond);
    printf(" }; \ndo\n { ");
    ast_pretty_print(node->body);
    printf(" }\ndone");
}

/* FOR LOOP PRINT */
static void for_loop_print(struct ast_node *toprint)
{
    struct for_loop_node *node = toprint->body.for_loop;
    if (!node)
    {
        printf("NULL");
        return;
    }
    printf("for %s", node->varname);
    if (node->args && node->args->size > 0)
    {
        printf(" in ");
        for (size_t i = 0; i < node->args->size; i++)
        {
            printf("%s", node->args->data[i]);
            if (i + 1 != node->args->size)
                printf(" ");
        }
    }
    printf("; \n do \n");
    ast_pretty_print(node->body);
    printf("\ndone\n");
}

/* NEG PRINT */
static void neg_print(struct ast_node *toprint)
{
    struct neg_node *node = toprint->body.negation;
    if (!node)
    {
        printf("NULL");
        return;
    }
    printf("! ");
    ast_pretty_print(node->elem);
}

static void var_print(struct ast_node *toprint)
{
    struct var_node *node = toprint->body.variable;
    if (!node)
    {
        printf("NULL");
        return;
    }
    printf("var %s", node->assignment);
}

static void func_print(struct ast_node *toprint)
{
    struct func_node *node = toprint->body.function;
    if (!node)
    {
        printf("NULL");
        return;
    }

    printf("%s()\n", node->name);
    ast_pretty_print(node->body);
    if (node->redir)
        ast_pretty_print(node->redir);
    printf("\n");
}

static void case_cmd_print(struct ast_node *toprint)
{
    struct case_node *node = toprint->body.case_cmd;
    if (!node)
    {
        printf("NULL");
        return;
    }

    printf("case %s in\n", node->var);

    struct ast_node *p = node->clause;

    while (p)
    {
        ast_pretty_print(p);
        p = p->next;
    }
    printf("esac");
}

static void case_item_print(struct ast_node *toprint)
{
    struct case_item_node *node = toprint->body.case_item;
    if (!node)
    {
        printf("NULL");
        return;
    }

    printf("( %s ", node->to_match->data[0]);
    for (size_t i = 1; i < node->to_match->size; i++)
        printf("| %s ", node->to_match->data[i]);
    printf(")\n");
    ast_pretty_print(node->body);
    printf("\n");
}

/* AST NODE PRINT JUMP TABLE */
typedef void (*ast_print_handler)(struct ast_node *node);

static const ast_print_handler print_jump_table[14] = {
    simple_cmd_print, if_cmd_print,   cmp_list_print,   redir_print,
    pipe_print,       and_or_print,   while_loop_print, until_loop_print,
    for_loop_print,   neg_print,      var_print,        func_print,
    case_cmd_print,   case_item_print
};

/* AST NODE PRINT */
void ast_pretty_print(struct ast_node *node)
{
    if (!node)
    {
        printf("NULL");
        return;
    }
    print_jump_table[node->type](node);
}
