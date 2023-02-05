#include "string_t.h"

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct string *string_init(size_t n)
{
    struct string *v = malloc(sizeof(struct string));
    if (!v)
        return NULL;
    char *d = calloc(n, sizeof(char));
    if (!d)
    {
        free(v);
        return NULL;
    }
    v->size = 0;
    v->capacity = n;
    v->data = d;
    return v;
}

void string_destroy(struct string *v)
{
    if (v)
        free(v);
}

struct string *string_resize(struct string *v, size_t n)
{
    if (n == v->capacity)
    {
        return v;
    }
    if (n < v->size)
    {
        v->size = n;
    }
    v->data = realloc(v->data, sizeof(char) * n);
    if (!v->data)
    {
        return NULL;
    }
    v->capacity = n;
    return v;
}

struct string *string_append(struct string *v, char elt)
{
    if (!v)
    {
        return NULL;
    }
    if (v->size + 1 > v->capacity)
    {
        v = string_resize(v, v->capacity * 2);
        if (!v)
        {
            return NULL;
        }
    }
    v->data[v->size] = elt;
    v->size++;
    return v;
}

struct string *string_create(char *str)
{
    struct string *new = string_init(1);
    for (int i = 0; str[i] != '\0'; i++)
        new = string_append(new, str[i]);
    new = string_append(new, '\0');
    return new;
}
