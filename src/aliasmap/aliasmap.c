#include "aliasmap.h"

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../parser/parser.h"

static struct aliasmap *als_g = NULL;
static int cnt = 0;

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

/* alias INIT */
static struct alias *alias_init(char *name, char *value)
{
    struct alias *new = malloc(sizeof(struct alias));
    if (!new)
        return new;
    new->name = name;
    new->value = value;
    return new;
}

/* alias DESTROY */
static void alias_destroy(struct alias *v)
{
    if (!v)
        return;
    if (v->name)
        free(v->name);
    if (v->value)
        free(v->value);
    free(v);
}

static void pair_list_al_destroy(struct pair_list_al *p)
{
    if (!p)
        return;
    alias_destroy(p->alias);
    free(p);
}

/* aliasmap INIT */
void aliasmap_init(size_t size)
{
    struct aliasmap *als = malloc(sizeof(struct aliasmap));
    if (als == NULL)
        return;
    als->size = size;
    als->data = malloc(size * sizeof(struct pair_list_al *));
    if (als->data == NULL)
    {
        free(als);
        return;
    }
    for (size_t i = 0; i < size; i++)
        als->data[i] = NULL;
    als_g = als;
}

/* aliasmap DESTROY */
void aliasmap_destroy(void)
{
    if (als_g == NULL)
        return;
    if (als_g->size != 0)
    {
        for (size_t i = 0; i < als_g->size; i++)
        {
            struct pair_list_al *p = als_g->data[i];
            struct pair_list_al *n;

            while (p != NULL)
            {
                n = p->next;
                pair_list_al_destroy(p);
                p = n;
            }
        }
    }
    free(als_g->data);
    als_g->size = 0;
    free(als_g);
}

void aliasmap_clear(void)
{
    if (als_g == NULL)
        return;
    if (als_g->size != 0)
    {
        for (size_t i = 0; i < als_g->size; i++)
        {
            struct pair_list_al *p = als_g->data[i];
            struct pair_list_al *n;

            while (p != NULL)
            {
                n = p->next;
                pair_list_al_destroy(p);
                p = n;
            }
        }
    }
}

int aliasmap_ass_insert(char *ass)
{
    char *name;
    char *value;
    parse_ass_word(ass, &name, &value);
    int err = aliasmap_insert(name, value);
    free(name);
    free(value);
    return err;
}

/* aliasmap INSERT */
int aliasmap_insert(char *name, char *value)
{
    if (als_g == NULL || als_g->size == 0)
    {
        return 0;
    }

    size_t h = hash(name);

    if (h > als_g->size)
        h %= als_g->size;

    if (als_g->data[h] != NULL)
    {
        // collision
        struct pair_list_al *p = als_g->data[h];
        while (p != NULL && strcmp(p->alias->name, name) != 0)
        {
            p = p->next;
        }
        if (p == NULL)
        {
            // key not present
            struct pair_list_al *q = malloc(sizeof(struct pair_list_al));
            if (q == NULL)
                return 0;
            q->alias = alias_init(strdup(name), strdup(value));
            cnt += 1;
            q->next = als_g->data[h];
            als_g->data[h] = q;
        }
        else
        {
            // key present
            // free(name);
            free(p->alias->value);
            p->alias->value = strdup(value);
        }
    }
    else
    {
        als_g->data[h] = malloc(sizeof(struct pair_list_al));
        if (als_g->data[h] == NULL)
            return 0;
        als_g->data[h]->alias = alias_init(strdup(name), strdup(value));
        cnt += 1;
        als_g->data[h]->next = NULL;
    }
    return 1;
}

/* aliasmap GET */
char *aliasmap_get(const char *aliasname)
{
    if (als_g == NULL || als_g->data == NULL)
        return NULL;
    for (size_t i = 0; i < als_g->size; i++)
    {
        for (struct pair_list_al *p = als_g->data[i]; p != NULL; p = p->next)
        {
            if (strcmp(p->alias->name, aliasname) == 0)
                return p->alias->value;
        }
    }
    return NULL;
}

/* aliasmap FIND */
static int aliasmap_find(const char *aliasname)
{
    if (als_g == NULL)
        return -1;
    for (size_t i = 0; i < als_g->size; i++)
    {
        for (struct pair_list_al *p = als_g->data[i]; p != NULL; p = p->next)
        {
            if (strcmp(p->alias->name, aliasname) == 0)
                return i;
        }
    }
    return -1;
}

/* aliasmap REMOVE */
int aliasmap_remove(char *aliasname)
{
    int i = aliasmap_find(aliasname);
    if (i == -1)
        return 0;

    struct pair_list_al *p = als_g->data[i];
    if (p->next == NULL)
    {
        als_g->data[i] = NULL;
        pair_list_al_destroy(p);
        cnt -= 1;
        return 1;
    }

    if (strcmp(p->alias->name, aliasname) == 0)
    {
        struct pair_list_al *tmp = p;
        p = p->next;
        pair_list_al_destroy(tmp);
        cnt -= 1;
        als_g->data[i] = p;
        return 1;
    }

    while (p->next != NULL)
    {
        if (strcmp(p->next->alias->name, aliasname) == 0)
        {
            struct pair_list_al *tmp = p->next;
            p->next = p->next->next;
            cnt -= 1;
            pair_list_al_destroy(tmp);
            return 1;
        }
        p = p->next;
    }
    return 0;
}

static int insert_ass_arr(char **arr, char *el, int len, int cap)
{
    if (len >= cap)
        return len;

    int i = len - 1;
    for (; i >= 0 && strcmp(arr[i], el) > 0; i--)
        arr[i + 1] = arr[i];

    arr[i + 1] = el;

    return len + 1;
}

void aliasmap_print(void)
{
    if (als_g == NULL)
        return;

    char **arr = calloc(cnt, sizeof(char *));
    int len = 0;

    for (size_t i = 0; i < als_g->size; i++)
    {
        if (als_g->data[i] != NULL)
        {
            struct pair_list_al *p = als_g->data[i];
            while (p != NULL)
            {
                len = insert_ass_arr(arr, p->alias->name, len, cnt);
                p = p->next;
            }
        }
    }

    for (int i = 0; i < len; i++)
    {
        printf("%s='%s'\n", arr[i], aliasmap_get(arr[i]));
    }
    free(arr);
}
