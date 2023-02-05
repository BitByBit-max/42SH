#include "parser.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../ast/ast.h"
#include "../token/token.h"
#include "../vector/vector.h"

static struct token *curr = NULL;
static int err = 0;

static int and_or(struct ast_node **node);

static int never_first(void)
{
    return curr->type == TOKEN_OR || curr->type == TOKEN_AND
        || curr->type == ELIF || curr->type == ELSE || curr->type == THEN
        || curr->type == TOKEN_PIPE || curr->type == TOKEN_DO
        || curr->type == TOKEN_DONE || curr->type == TOKEN_DONE
        || curr->type == SCOL || curr->type == AMPERSAND || curr->type == NEWL
        || curr->type == END || curr->type == DOUBLE_SCOL;
}

static enum redir_type redirect_type(void)
{
    if (curr->type == REDIR_DMORE)
        return DMORE;
    else if (curr->type == REDIR_LESS)
        return LESS;
    else if (curr->type == REDIR_LESSAND)
        return LESSAND;
    else if (curr->type == REDIR_LESSMORE)
        return LESSMORE;
    else if (curr->type == REDIR_MORE)
        return MORE;
    else if (curr->type == REDIR_MOREAND)
        return MOREAND;
    return MOREOR;
}

static int shell_cmd(void)
{
    return curr->type == TOKEN_UNTIL || curr->type == TOKEN_FOR
        || curr->type == TOKEN_WHILE || curr->type == IF
        || curr->type == OPENED_P || curr->type == OPEN_CB
        || curr->type == CASE;
}

static enum ast_type shell_type(void)
{
    if (curr->type == TOKEN_UNTIL)
        return UNTIL_LOOP;
    if (curr->type == TOKEN_WHILE)
        return WHILE_LOOP;
    if (curr->type == TOKEN_FOR)
        return FOR_LOOP;
    if (curr->type == IF)
        return IF_CMD;
    if (curr->type == CASE)
        return CASE_CMD;
    return CMP_LIST;
}

static enum cmp_type cmp_list_type(void)
{
    if (curr->type == OPEN_CB)
        return CMD_BLOCK;
    if (curr->type == OPENED_P)
        return SUBSHELL;
    return LIST;
}

static enum pipe_type and_or_type(void)
{
    if (curr->type == TOKEN_AND)
        return AND;
    return OR;
}

static int redirect(void)
{
    return curr->type > 10 && curr->type < 18;
}

static void next(int reserved, int io, int alias) // 1 means all words
{
    token_destroy(curr);
    curr = lexing(reserved, io, alias, &err);
}

static void skip_newl(void)
{
    while (curr->type == NEWL)
        next(0, 1, 1);
}

static int redirection(struct ast_node **node)
{
    enum type_t ptype = curr->type;
    (*node) = ast_node_init(REDIR);
    (*node)->body.redir->src = redir_file_init();
    if (curr->type == IONUMBER)
    {
        (*node)->body.redir->src->fd = atoi(curr->data); // we need an int
        next(0, 1, 1);
    }
    else
        (*node)->body.redir->src->fd = 1;
    if (!redirect())
        return ptype == IONUMBER ? ERROR : NO_MATCH;
    if (!*node)
        (*node) = ast_node_init(REDIR);
    (*node)->body.redir->type = redirect_type();
    next(1, 0, 1);
    if (curr->type != WORD)
        return ERROR;
    (*node)->body.redir->dest = redir_file_init();
    (*node)->body.redir->dest->path = strdup(curr->data);
    next(0, 1, 1);
    return DONE;
}

static struct ast_node *get_last_prefix(struct ast_node *prefixes)
{
    struct ast_node *pre = prefixes;
    while (pre->type == REDIR && pre->body.redir->elem)
        pre = pre->body.redir->elem;
    return pre;
}

static int parse_element(void)
{
    return redirect() || curr->type == WORD || curr->type == IONUMBER;
}

static int element(struct ast_node **node)
{
    if (curr->type == WORD)
    {
        struct vector *v = (*node)->body.simple_cmd->args;
        if (!v)
            (*node)->body.simple_cmd->args = vector_init(1);
        v = (*node)->body.simple_cmd->args;
        v = vector_append(v, strdup(curr->data));
        next(1, 1, 1);
        return DONE;
    }
    if (curr->type == IONUMBER || redirect())
        return redirection(node);
    return NO_MATCH;
}

static int many_elem(struct ast_node **node, struct ast_node *comm)
{
    struct ast_node *main_redir = NULL;
    struct ast_node **redir = &main_redir;
    while (parse_element())
    {
        if (curr->type == WORD)
            element(&comm);
        else if (curr->type == IONUMBER || redirect())
        {
            if (redirection(redir) == ERROR)
            {
                ast_node_destroy(comm);
                ast_node_destroy(*redir);
                return ERROR;
            }
            redir = &((*redir)->body.redir->elem);
        }
        else
            comm->body.simple_cmd->args = vector_init(1);
    }
    if (!(comm)->body.simple_cmd->args)
        (comm)->body.simple_cmd->args = vector_init(1);
    if (main_redir)
    {
        *node = main_redir;
        *redir = get_last_prefix(main_redir);
        (*redir)->body.redir->elem = comm;
    }
    return DONE;
}

static int many_prefix(struct ast_node **node)
{
    struct ast_node *main_redir = NULL;
    struct ast_node **redir = &main_redir;
    struct ast_node *vars = NULL;
    struct ast_node **next_vars = &vars;
    while (curr->type == IONUMBER || redirect() || curr->type == ASS_WORD)
    {
        if (curr->type == ASS_WORD)
        {
            if (!vars)
                vars = ast_node_init(VAR);
            (*next_vars)->body.variable->assignment = strdup(curr->data);
            next_vars = &((*next_vars)->next);
            next(0, 1, 1);
        }
        else
        {
            if (redirection(redir) == ERROR)
            {
                ast_node_destroy(*redir);
                return ERROR;
            }
            redir = &((*redir)->body.redir->elem);
        }
    }
    if (main_redir)
        *node = main_redir;
    if (vars)
    {
        if (*node)
            (*node)->next = vars;
        else
            *node = vars;
    }
    return DONE;
}

static int simp_cmd(struct ast_node **node, char *command)
{
    struct ast_node *prefixes = NULL;
    if (many_prefix(&prefixes) == ERROR)
        return ERROR;
    if (curr->type != WORD && !command)
    {
        *node = prefixes;
        return DONE;
    }
    struct ast_node *comm = ast_node_init(SIMPLE_CMD);
    comm->body.simple_cmd->cmd = command ? strdup(command) : strdup(curr->data);
    if (!command)
    {
        if (!strcmp(curr->data, "unalias"))
            next(1, 1, 0);
        else
            next(1, 1, 1);
    }
    free(command);
    struct ast_node *elements = NULL;
    if (parse_element())
    {
        if (many_elem(&elements, comm) == ERROR)
            return ERROR;
    }
    if (prefixes)
    {
        if (prefixes->type == REDIR)
        {
            struct ast_node *last_prefix = get_last_prefix(prefixes);
            if (elements)
                last_prefix->body.redir->elem = elements;
            else
                last_prefix->body.redir->elem = comm;
            *node = prefixes;
            comm->body.simple_cmd->vars = prefixes->next;
        }
        if (prefixes->type == VAR)
            comm->body.simple_cmd->vars = prefixes;
        if (!comm->body.simple_cmd->args)
            comm->body.simple_cmd->args = vector_init(1);
    }
    else if (elements)
        *node = elements;
    else
    {
        if (!(comm)->body.simple_cmd->args)
            (comm)->body.simple_cmd->args = vector_init(1);
        *node = comm;
    }
    return DONE;
}

static int compound_list(struct ast_node **node)
{
    skip_newl();
    *node = ast_node_init(CMP_LIST);
    (*node)->body.cmp_list->type = LIST;
    if (and_or((&(*node)->body.cmp_list->elem)) == ERROR)
        return ERROR;
    struct ast_node *n = (*node)->body.cmp_list->elem;
    struct ast_node *next_n = NULL;
    while (curr->type == SCOL || curr->type == NEWL)
    {
        if (curr->type == SCOL || curr->type == NEWL)
            next(0, 1, 1);
        skip_newl();
        and_or(&next_n);
        if (n && next_n)
        {
            n->next = next_n;
            next_n = next_n->next;
            n = n->next;
        }
    }
    if (curr->type == SCOL)
        next(0, 1, 1);
    skip_newl();
    return DONE;
}

static int init_cmp_list(struct ast_node **node)
{
    *node = ast_node_init(CMP_LIST);
    (*node)->body.cmp_list->type = cmp_list_type();
    if (curr->type == OPENED_P || curr->type == OPEN_CB)
    {
        next(0, 1, 1);
        skip_newl();
    }
    if (compound_list(&(*node)->body.cmp_list->elem) == ERROR)
        return ERROR;
    if ((*node)->body.cmp_list->type == CMD_BLOCK && curr->type != CLOSED_CB)
        return ERROR;
    if ((*node)->body.cmp_list->type == SUBSHELL && curr->type != CLOSED_P)
        return ERROR;
    if (curr->type == CLOSED_P || curr->type == CLOSED_CB)
        next(0, 1, 1);
    return DONE;
}

static int else_clause(struct ast_node **node)
{
    if (curr->type == ELSE)
    {
        next(0, 1, 1);
        if (!compound_list(node))
            return NO_MATCH;
    }
    else if (curr->type == ELIF)
    {
        (*node) = ast_node_init(IF_CMD);
        next(0, 1, 1);
        if (compound_list(&(*node)->body.if_cmd->cond) == ERROR)
            return ERROR;
        if (curr->type != THEN)
            return ERROR;
        next(0, 1, 1);
        if (compound_list(&(*node)->body.if_cmd->cond_true) == ERROR)
            return ERROR;
        if (curr->type == ELSE || curr->type == ELIF)
            else_clause(&(*node)->body.if_cmd->cond_false);
    }
    return DONE;
}

static int rule_if(struct ast_node **node)
{
    next(0, 1, 1);
    if (compound_list(&(*node)->body.if_cmd->cond) == ERROR)
        return ERROR;
    if (curr->type != THEN)
        return ERROR;
    next(0, 1, 1);
    if (compound_list(&(*node)->body.if_cmd->cond_true) == ERROR)
        return ERROR;
    if (curr->type == ELSE || curr->type == ELIF)
    {
        if (else_clause(&(*node)->body.if_cmd->cond_false) == ERROR)
            return ERROR;
    }
    if (curr->type != FI)
        return ERROR;
    next(0, 1, 1);
    return DONE;
}

static int rule_until(struct ast_node **node)
{
    next(0, 1, 1);
    if (compound_list(&(*node)->body.until_loop->cond) == ERROR)
        return ERROR;
    if (curr->type != TOKEN_DO)
        return ERROR;
    next(0, 1, 1);
    if (compound_list(&(*node)->body.until_loop->body) == ERROR)
        return ERROR;
    if (curr->type != TOKEN_DONE)
        return ERROR;
    next(0, 1, 1);
    return DONE;
}

static int rule_while(struct ast_node **node)
{
    next(0, 1, 1);
    if (compound_list(&(*node)->body.while_loop->cond) == ERROR)
        return ERROR;
    if (curr->type != TOKEN_DO)
        return ERROR;
    next(0, 1, 1);
    if (compound_list(&(*node)->body.while_loop->body) == ERROR)
        return ERROR;
    if (curr->type != TOKEN_DONE)
        return ERROR;
    next(0, 1, 1);
    return DONE;
}

static int rule_for(struct ast_node **node)
{
    next(0, 0, 1);
    if (curr->type != WORD)
        return ERROR;
    (*node)->body.for_loop->varname = strdup(curr->data);
    (*node)->body.for_loop->args = vector_init(1);
    next(0, 0, 1);
    if (curr->type == SCOL)
        next(0, 0, 1);
    else
    {
        skip_newl();
        if (curr->type == TOKEN_IN)
        {
            next(1, 0, 1);
            while (curr->type == WORD)
            {
                vector_append((*node)->body.for_loop->args, strdup(curr->data));
                next(1, 0, 1);
            }
            if (curr->type != SCOL && curr->type != NEWL)
                return ERROR;
            next(0, 0, 1);
        }
    }
    skip_newl();
    if (curr->type != TOKEN_DO)
        return ERROR;
    next(0, 1, 1);
    if (compound_list(&(*node)->body.for_loop->body) == ERROR)
        return ERROR;
    if (curr->type != TOKEN_DONE)
        return ERROR;
    next(0, 1, 1);
    return DONE;
}

static void skip_dscol(void)
{
    while (curr->type == DOUBLE_SCOL)
        next(0, 1, 1);
}

static int parse_case_item(struct ast_node **node)
{
    if (curr->type == OPENED_P)
        next(0, 0, 1);
    if (curr->type != WORD)
        return ERROR;
    *node = ast_node_init(CASE_ITEM);
    (*node)->body.case_item->to_match = vector_init(1);
    vector_append((*node)->body.case_item->to_match, strdup(curr->data));
    next(1, 0, 1);
    while (curr->type == TOKEN_PIPE)
    {
        next(0, 0, 1);
        vector_append((*node)->body.case_item->to_match, strdup(curr->data));
        next(1, 0, 1);
    }
    if (curr->type != CLOSED_P)
        return ERROR;
    if (curr->type == NEWL)
        skip_newl();
    else
        next(0, 1, 1);
    if (compound_list(&(*node)->body.case_item->body) == ERROR)
        return ERROR;
    return DONE;
}

static int case_clause(struct ast_node **node)
{
    if (parse_case_item(node) == ERROR)
        return ERROR;
    struct ast_node **next_node = &((*node)->next);
    while (curr->type == DOUBLE_SCOL)
    {
        next(0, 1, 1);
        skip_newl();
        if (curr->type == ESAC)
            break;
        if (parse_case_item(next_node) == ERROR)
            return ERROR;
        if (*next_node)
            next_node = &((*next_node)->next);
    }
    skip_dscol();
    skip_newl();
    return DONE;
}

static int rule_case(struct ast_node **node)
{
    next(0, 0, 1);
    if (curr->type != WORD)
        return ERROR;
    (*node)->body.case_cmd->var = strdup(curr->data);
    if (curr->type == NEWL)
        skip_newl();
    else
        next(0, 1, 1);
    if (curr->type != TOKEN_IN)
        return ERROR;
    next(0, 1, 1);
    skip_newl();
    if (case_clause(&(*node)->body.case_cmd->clause) == ERROR)
        return ERROR;
    if (curr->type != ESAC)
        return ERROR;
    next(0, 1, 1);
    return DONE;
}

static int shell_command(struct ast_node **node)
{
    if (curr->type == IF)
        return rule_if(node);
    else if (curr->type == TOKEN_WHILE)
        return rule_while(node);
    else if (curr->type == TOKEN_UNTIL)
        return rule_until(node);
    else if (curr->type == TOKEN_FOR)
        return rule_for(node);
    else if (curr->type == OPENED_P || curr->type == OPEN_CB)
        return init_cmp_list(node);
    else if (curr->type == CASE)
        return rule_case(node);
    return NO_MATCH;
}

static int parse_shell_cmd(struct ast_node **func)
{
    struct ast_node *shell_cmd = NULL;
    if (shell_type() != CMP_LIST)
        shell_cmd = ast_node_init(shell_type());
    if (shell_command(&shell_cmd) == ERROR)
    {
        ast_node_destroy(*func);
        ast_node_destroy(shell_cmd);
        return ERROR;
    }
    (*func)->body.function->body = shell_cmd;
    return DONE;
}

static int declare_func(struct ast_node **func)
{
    skip_newl();
    if (shell_cmd())
        return parse_shell_cmd(func);
    return ERROR;
}

static int many_redir(struct ast_node **node, struct ast_node *shell_cmd)
{
    struct ast_node *main_redir = NULL;
    struct ast_node *redirections = NULL;
    int res = -1;
    while (redirect() || curr->type == IONUMBER)
    {
        if (!main_redir)
        {
            res = redirection(&redirections); // this is the head
            main_redir = redirections;
        }
        else
        {
            res = redirection(&redirections->body.redir->elem);
            if (res != DONE)
                break;
            redirections = redirections->body.redir->elem;
        }
    }
    *node = main_redir;
    if (main_redir)
        redirections->body.redir->elem = shell_cmd;
    return main_redir != NULL;
}

static int parsing_func_or_simp(struct ast_node **node)
{
    char *saved = strdup(curr->data);
    if (!strcmp(curr->data, "unalias"))
        next(1, 1, 0);
    else
        next(1, 1, 1);
    if (curr->type == OPENED_P)
    {
        next(0, 0, 1);
        if (curr->type != CLOSED_P)
        {
            free(saved);
            return ERROR;
        }
        next(0, 1, 1);
        struct ast_node *func = ast_node_init(FUNC);
        func->body.function->name = strdup(saved);
        free(saved);
        if (declare_func(&func) == ERROR)
        {
            ast_node_destroy(func);
            return ERROR;
        }
        *node = func;
        struct ast_node *redirs = NULL;
        if (many_redir(&redirs, NULL))
            (*node)->body.function->redir = redirs;
    }
    else if (simp_cmd(node, saved) == ERROR)
        return ERROR;
    return DONE;
}

static int parsing_shell_cmd(struct ast_node **node)
{
    struct ast_node *shell_cmd = NULL;
    if (shell_type() != CMP_LIST)
        shell_cmd = ast_node_init(shell_type());
    if (shell_command(&shell_cmd) == ERROR)
    {
        ast_node_destroy(shell_cmd);
        return ERROR;
    }
    if (!many_redir(node, shell_cmd))
        *node = shell_cmd;
    return DONE;
}

static int command(struct ast_node **node)
{
    if (curr->type == IONUMBER || redirect() || curr->type == ASS_WORD)
        return simp_cmd(node, NULL);
    else if (curr->type == WORD)
        return parsing_func_or_simp(node);
    else if (shell_cmd())
        return parsing_shell_cmd(node);
    else if (never_first())
        return ERROR;
    return NO_MATCH;
}

static int many_pipes(struct ast_node *first, struct ast_node **node)
{
    struct ast_node *comm = NULL;
    struct ast_node *head = NULL;
    struct ast_node **pipe = &head;
    struct ast_node *newpipe = NULL;
    while (curr->type == TOKEN_PIPE)
    {
        if (!head)
        {
            head = ast_node_init(PIPE);
            head->body.pipe->elem = first;
        }
        else
        {
            newpipe = ast_node_init(PIPE);
            newpipe->body.pipe->elem = comm;
        }
        if (curr->type == NEWL)
            skip_newl();
        else
            next(0, 1, 1);

        struct ast_node *curr_comm = NULL;
        if (command(&curr_comm) == ERROR)
        {
            ast_node_destroy(head);
            ast_node_destroy(newpipe);
            ast_node_destroy(curr_comm);
            return ERROR;
        }

        if (newpipe)
        {
            (*pipe)->body.pipe->elem->next = newpipe;
            pipe = &((*pipe)->body.pipe->elem->next);
        }

        if (curr_comm && curr->type != TOKEN_PIPE)
            (*pipe)->body.pipe->elem->next = curr_comm;
        comm = curr_comm;
    }
    *node = head;
    return DONE;
}

static int pipeline(struct ast_node **node)
{
    struct ast_node *neg = NULL;
    if (curr->type == NEGATION)
    {
        neg = ast_node_init(NEG);
        next(0, 1, 1);
    }
    struct ast_node *comm = NULL;
    int cmd_res;
    if ((cmd_res = command(&comm)) == ERROR)
    {
        ast_node_destroy(comm);
        ast_node_destroy(neg);
        return cmd_res;
    }
    struct ast_node *main_pipe = NULL;
    if (curr->type == TOKEN_PIPE)
    {
        if (many_pipes(comm, &main_pipe) == ERROR)
        {
            ast_node_destroy(main_pipe);
            return ERROR;
        }
    }
    if (neg)
    {
        neg->body.negation->elem = main_pipe != NULL ? main_pipe : comm;
        *node = neg;
    }
    else
        *node = main_pipe != NULL ? main_pipe : comm;
    return DONE;
}

static int many_and_or(struct ast_node *first, struct ast_node **node)
{
    struct ast_node *comm = NULL;
    struct ast_node *and_or = NULL;
    struct ast_node *head = NULL;
    while (curr->type == TOKEN_AND || curr->type == TOKEN_OR)
    {
        if (!and_or)
        {
            and_or = ast_node_init(AND_OR);
            and_or->body.and_or->type = and_or_type();
            and_or->body.and_or->left = first;
        }
        else
        {
            and_or = ast_node_init(AND_OR);
            and_or->body.and_or->type = and_or_type();
            and_or->body.and_or->left = head;
        }

        if (curr->type == NEWL)
            skip_newl();
        else
            next(0, 1, 1);

        if (pipeline(&comm) == ERROR)
        {
            ast_node_destroy(head);
            return ERROR;
        }

        head = and_or;
        head->body.and_or->right = comm;
    }
    *node = head;
    return DONE;
}

static int and_or(struct ast_node **node)
{
    struct ast_node *pipe = NULL;
    if (pipeline(&pipe) == ERROR)
    {
        ast_node_destroy(pipe);
        return ERROR;
    }
    struct ast_node *and_or = NULL;
    if (curr->type == TOKEN_AND || curr->type == TOKEN_OR)
    {
        if (many_and_or(pipe, &and_or) == ERROR)
            return ERROR;
    }
    *node = and_or ? and_or : pipe;
    return DONE;
}

static int list(struct ast_node **node)
{
    (*node) = ast_node_init(CMP_LIST);
    (*node)->body.cmp_list->type = LIST;
    if (and_or(&(*node)->body.cmp_list->elem) == ERROR)
        return ERROR;

    struct ast_node *n = (*node)->body.cmp_list->elem;
    struct ast_node *next_node = NULL;
    while (curr->type == SCOL)
    {
        next(0, 1, 1);
        if (curr->type == END || curr->type == NEWL)
            break;
        if (and_or(&next_node) == ERROR)
            return ERROR;
        //        if (!next_node)
        //          break;
        if (n)
        {
            n->next = next_node;
            n = n->next;
        }
    }
    return DONE;
}

static enum parser_status input(struct ast_node **node)
{
    if (curr->type == END)
        return NO_MATCH;
    if (curr->type == NEWL)
        return 3;
    return list(node);
}

static struct ast_node *parse(int *error)
{
    struct ast_node *node = NULL;
    curr = lexing(0, 1, 1, &err);
    int e;
    if ((e = input(&node)) == ERROR || err == 2) // err lexing
    {
        *error = 2;
        ast_node_destroy(node);
        token_destroy(curr);
        return NULL;
    }
    if (e == 3)
        *error = 3;
    token_destroy(curr);
    return node;
}

struct ast_node *build(int *error)
{
    return parse(error);
}

void parse_ass_word(char *ass, char **name, char **value)
{
    if (!ass || ass[0] == '\0')
        return;

    size_t i = 0;
    while (ass[i] != '\0' && ass[i] != '=')
        i += 1;
    *name = strndup(ass, i);
    if (ass[i] != '=')
        return;
    i += 1;
    int sq = ass[i] == '\'';
    int dq = ass[i] == '\"';
    i += sq + dq;
    size_t start = i;
    while (ass[i] != '\0' && ass[i] != '\"' && ass[i] != '\'')
        i += 1;
    *value = strndup(ass + start, i - start);
}

void parse_subshell(char *ass, char **subshell, char **toreplace, int dollar)
{
    if (!ass || ass[0] == '\0')
        return;
    if (dollar == 1)
    {
        size_t i = 0;
        while (ass[i] != '\0' && ass[i] != '$' && ass[i + 1] != '(')
            i += 1;

        size_t to_start = i;
        if (ass[i] == '\0')
            return;

        i += 2; // skip (

        size_t start = i;
        int nb = 1;
        while (ass[i] != '\0')
        {
            nb += ass[i] == '(';
            nb -= ass[i] == ')';
            if (nb == 0)
                break;
            i += 1;
        }
        *subshell = strndup(ass + start, i - start);
        *toreplace = strndup(ass + to_start, i - to_start + 1);
    }
    else
    {
        size_t i = 0;
        while (ass[i] != '\0' && ass[i] != '`')
            i += 1;

        size_t to_start = i;
        if (ass[i] == '\0')
            return;

        size_t start = i;
        char prev = ass[i];
        i += 1;
        while (ass[i] != '\0' && !(ass[i] == '`' && prev != '\\'))
        {
            prev = ass[i];
            i += 1;
        }
        *subshell = strndup(ass + start + 1, i - start - 1);
        *toreplace = strndup(ass + to_start, i - to_start + 1);
    }
}
