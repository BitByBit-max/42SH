#ifndef STRING_T_H
#define STRING_T_H

#include <stddef.h>

struct string
{
    // The number of elements in the string
    size_t size;
    // The maximum number of elements in the string
    size_t capacity;
    // The elements themselves
    char *data;
};

/*
** Initialize the string with `n` capacity.
** Returns `NULL` if an error occured.
*/
struct string *string_init(size_t n);

/*
** Release the memory used by the string.
** Does nothing if `v` is `NULL`.
*/
void string_destroy(struct string *v);

/*
** Resize the string to `n` capacity.
** Returns `NULL` if an error occured.
*/
struct string *string_resize(struct string *v, size_t n);

/*
** Append an element to the end of the string. Expand the string if needed.
** Returns `NULL` if an error occured.
*/
struct string *string_append(struct string *v, char elt);

struct string *string_create(char *str);

#endif /* ! STRING_T_H */
