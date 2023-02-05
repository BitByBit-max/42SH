#ifndef ALIASMAP_H
#define ALIASMAP_H

#include <stddef.h>

struct alias
{
    char *name;
    char *value;
};

struct pair_list_al
{
    struct alias *alias;
    struct pair_list_al *next;
};

struct aliasmap
{
    struct pair_list_al **data;
    size_t size;
};

/* aliasmap INIT */
void aliasmap_init(size_t size);

/* aliasmap DESTROY */
void aliasmap_destroy(void);

/* ALIASMAP CLEAR */
void aliasmap_clear(void);

/* aliasmap REMOVE */
int aliasmap_remove(char *aliasname);

int aliasmap_ass_insert(char *ass);

/* aliasmap INSERT */
int aliasmap_insert(char *name, char *value);

/* aliasmap GET */
char *aliasmap_get(const char *aliasname);

/* aliasmap PRINT */
void aliasmap_print(void);

#endif /* ! ALIASMAP_H */
