#include "vector.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct vector *vector_init(size_t n)
{
    struct vector *vec = malloc(sizeof(struct vector));
    if (!vec)
        return NULL;
    vec->capacity = n;
    vec->size = 0;
    vec->data = malloc(sizeof(char *) * n);
    if (!vec->data)
        return NULL;
    return vec;
}

void vector_destroy(struct vector *v)
{
    if (!v)
        return;
    if (v->data)
    {
        for (size_t i = 0; i < v->size; i++) // was v->capacity but segv
            free(v->data[i]);
        free(v->data);
    }
    free(v);
}

struct vector *vector_resize(struct vector *v, size_t n)
{
    if (v->capacity > n)
    {
        if (v->size > n)
            v->size = n;

        v->capacity = n;
        v->data = realloc(v->data, sizeof(char *) * n);
    }
    else if (v->capacity < n)
    {
        v->capacity *= 2;
        v->data = realloc(v->data, sizeof(char *) * v->capacity);
    }
    else if (v->size < v->capacity / 2)
    {
        v->capacity /= 2;
        v->data = realloc(v->data, sizeof(char *) * v->capacity);
    }
    return v;
}

struct vector *vector_append(struct vector *v, char *elt)
{
    if (!v)
        return NULL;
    if (v->size >= v->capacity)
        v = vector_resize(v, v->size + 1);
    v->data[v->size] = elt;
    v->size++;
    return v;
}

void vector_print(const struct vector *v)
{
    if (v && v->size != 0)
    {
        size_t i = 0;
        for (; i < (v->size - 1); i++)
            printf("%s,", v->data[i]);
        printf("%s", v->data[i]);
    }
    printf("\n");
}

struct vector *vector_insert(struct vector *v, size_t i, char *elt)
{
    if (v == NULL || i > v->size)
        return NULL;

    if (v->size + 1 >= v->capacity)
    {
        v = vector_resize(v, v->size + 1);
        if (v == NULL)
            return NULL;
    }

    size_t j = v->size;
    v->data[j] = elt;

    size_t r = v->size - i;
    while (r > 0)
    {
        char *tmp = v->data[j];
        v->data[j] = v->data[j - 1];
        v->data[j - 1] = tmp;
        j -= 1;
        r -= 1;
    }
    v->size += 1;
    return v;
}
