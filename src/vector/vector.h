#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

struct vector
{
    // The number of elements in the vector
    size_t size;
    // The maximum number of elements in the vector
    size_t capacity;
    // The elements themselves
    char **data;
};

/*
** Initialize the vector with `n` capacity.
** Returns `NULL` if an error occured.
*/
struct vector *vector_init(size_t n);

/*
** Release the memory used by the vector.
** Does nothing if `v` is `NULL`.
*/
void vector_destroy(struct vector *v);

/*
** Resize the vector to `n` capacity.
** Returns `NULL` if an error occured.
*/
struct vector *vector_resize(struct vector *v, size_t n);

/*
** Append an element to the end of the vector. Expand the vector if needed.
** Returns `NULL` if an error occured.
*/
struct vector *vector_append(struct vector *v, char *elt);

/*
** Display the vector contents on `stdout`.
** Displays `\n` if `v` is `NULL`.
*/
void vector_print(const struct vector *v);

struct vector *vector_insert(struct vector *v, size_t i, char *elt);

#endif /* ! VECTOR_H */
