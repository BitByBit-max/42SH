#include "funcmap.h"

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ast/ast.h"

static struct funcmap *funcs_g = NULL;

/* HASH */
static size_t hash(const char *key)
{
    size_t i = 0;
    size_t hash = 0;

    for (i = 0; key[i] != '\0'; ++i)
        hash += key[i];
    hash += i;

    return hash;
}

/* func INIT */
static struct func *func_init(char *name, struct ast_node *body)
{
    struct func *new = calloc(1, sizeof(struct func));
    if (!new)
        return new;
    new->name = name;
    new->body = body;
    return new;
}

/* func DESTROY */
static void func_destroy(struct func *v)
{
    if (!v)
        return;
    if (v->name)
        free(v->name);
    ast_node_destroy(v->body);
    free(v);
}

static void pair_list_destroy(struct pair_list_func *p)
{
    if (!p)
        return;
    func_destroy(p->func);
    free(p);
}

/* funcmap INIT */
void funcmap_init(size_t size)
{
    funcs_g = malloc(sizeof(struct funcmap));
    if (funcs_g == NULL)
        return;
    funcs_g->size = size;
    funcs_g->data = malloc(size * sizeof(struct pair_list_func *));
    if (funcs_g->data == NULL)
        return;
    for (size_t i = 0; i < size; i++)
        funcs_g->data[i] = NULL;
}

/* funcmap DESTROY */
void funcmap_destroy(void)
{
    if (funcs_g == NULL)
        return;
    if (funcs_g->size != 0)
    {
        for (size_t i = 0; i < funcs_g->size; i++)
        {
            struct pair_list_func *p = funcs_g->data[i];
            struct pair_list_func *n;

            while (p != NULL)
            {
                n = p->next;
                pair_list_destroy(p);
                p = n;
            }
        }
    }
    free(funcs_g->data);
    funcs_g->size = 0;
    free(funcs_g);
}

/* funcmap INSERT */
int funcmap_insert(char *name, struct ast_node *body)
{
    if (funcs_g == NULL || funcs_g->size == 0)
    {
        return 0;
    }

    size_t h = hash(name);

    if (h > funcs_g->size)
        h %= funcs_g->size;

    if (funcs_g->data[h] != NULL)
    {
        // collision
        struct pair_list_func *p = funcs_g->data[h];
        while (p != NULL && strcmp(p->func->name, name) != 0)
        {
            p = p->next;
        }
        if (p == NULL)
        {
            // key not present
            struct pair_list_func *q = malloc(sizeof(struct pair_list_func));
            if (q == NULL)
                return 0;
            q->func = func_init(strdup(name), ast_dup(body));
            q->next = funcs_g->data[h];
            funcs_g->data[h] = q;
        }
        else
        {
            // key present
            ast_node_destroy(p->func->body);
            p->func->body = ast_dup(body);
        }
    }
    else
    {
        funcs_g->data[h] = malloc(sizeof(struct pair_list_func));
        if (funcs_g->data[h] == NULL)
            return 0;
        funcs_g->data[h]->func = func_init(strdup(name), ast_dup(body));
        funcs_g->data[h]->next = NULL;
    }
    return 1;
}

/* funcmap GET */
struct func *funcmap_get(const char *funcname)
{
    if (funcs_g == NULL || funcs_g->data == NULL)
        return NULL;
    for (size_t i = 0; i < funcs_g->size; i++)
    {
        for (struct pair_list_func *p = funcs_g->data[i]; p != NULL;
             p = p->next)
        {
            if (strcmp(p->func->name, funcname) == 0)
                return p->func;
        }
    }
    return NULL;
}

/* FUNCMAP FIND */
static int funcmap_find(const char *name)
{
    if (funcs_g == NULL)
        return -1;
    for (size_t i = 0; i < funcs_g->size; i++)
    {
        for (struct pair_list_func *p = funcs_g->data[i]; p != NULL;
             p = p->next)
        {
            if (strcmp(p->func->name, name) == 0)
                return i;
        }
    }
    return -1;
}

/* FUNCMAP REMOVE */
int funcmap_remove(char *funcname)
{
    int i = funcmap_find(funcname);
    if (i == -1)
        return 0;

    struct pair_list_func *p = funcs_g->data[i];
    if (p->next == NULL)
    {
        funcs_g->data[i] = NULL;
        pair_list_destroy(p);
        return 1;
    }

    if (strcmp(p->func->name, funcname) == 0)
    {
        struct pair_list_func *tmp = p;
        p = p->next;
        pair_list_destroy(tmp);
        funcs_g->data[i] = p;
        return 1;
    }

    while (p->next != NULL)
    {
        if (strcmp(p->next->func->name, funcname) == 0)
        {
            struct pair_list_func *tmp = p->next;
            p->next = p->next->next;
            pair_list_destroy(tmp);
            return 1;
        }
        p = p->next;
    }
    return 0;
}

void funcmap_print(void)
{
    if (funcs_g == NULL)
        return;
    printf("\n ======= funcmap PRINT ====== \n");
    for (size_t i = 0; i < funcs_g->size; i++)
    {
        if (funcs_g->data[i] != NULL)
        {
            size_t count = 0;
            struct pair_list_func *p = funcs_g->data[i];
            while (p != NULL)
            {
                p = p->next;
                count += 1;
            }

            p = funcs_g->data[i];
            size_t i = 0;
            while (i < count)
            {
                printf("%s()\n", p->func->name);
                ast_pretty_print(p->func->body);
                i += 1;
                p = p->next;
            }
            printf("\n");
        }
    }
    printf("\n ======= funcmap PRINT ====== \n");
}
