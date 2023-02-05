#include "varmap.h"

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../parser/parser.h"
#include "special_vars.h"

static struct varmap *vars_g = NULL;

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

/* VAR INIT */
static struct var *var_init(char *name, char *value)
{
    struct var *new = calloc(1, sizeof(struct var));
    if (!new)
        return new;
    new->name = name;
    new->value = value;
    return new;
}

/* VAR DESTROY */
static void var_destroy(struct var *v)
{
    if (!v)
        return;
    if (v->name)
        free(v->name);
    if (v->value)
        free(v->value);
    free(v);
}

static void pair_list_destroy(struct pair_list *p)
{
    if (!p)
        return;
    var_destroy(p->var);
    free(p);
}

/* VARMAP INIT */
struct varmap *varmap_init(size_t size)
{
    struct varmap *vars = malloc(sizeof(struct varmap));
    if (vars == NULL)
        return NULL;
    vars->size = size;
    vars->data = malloc(size * sizeof(struct pair_list *));
    if (vars->data == NULL)
    {
        free(vars);
        return NULL;
    }
    for (size_t i = 0; i < size; i++)
        vars->data[i] = NULL;
    if (!vars_g)
        vars_g = vars;
    return vars;
}

/* VARMAP DESTROY */
void varmap_destroy(struct varmap *vars)
{
    if (vars == NULL)
        return;
    if (vars->size != 0)
    {
        for (size_t i = 0; i < vars->size; i++)
        {
            struct pair_list *p = vars->data[i];
            struct pair_list *n;

            while (p != NULL)
            {
                n = p->next;
                pair_list_destroy(p);
                p = n;
            }
        }
    }
    free(vars->data);
    vars->size = 0;
    free(vars);
}

int varmap_insert_ass(struct varmap *vars, char *assingment)
{
    char *name = NULL;
    char *value = NULL;
    parse_ass_word(assingment, &name, &value);
    int r = varmap_insert(vars, name, value);
    free(name);
    free(value);
    return r;
}

/* VARMAP INSERT */
int varmap_insert(struct varmap *vars, char *name, char *value)
{
    if (vars == NULL || vars->size == 0)
    {
        return 0;
    }

    size_t h = hash(name);

    if (h > vars->size)
        h %= vars->size;

    if (vars->data[h] != NULL)
    {
        // collision
        struct pair_list *p = vars->data[h];
        while (p != NULL && strcmp(p->var->name, name) != 0)
        {
            p = p->next;
        }
        if (p == NULL)
        {
            // key not present
            struct pair_list *q = malloc(sizeof(struct pair_list));
            if (q == NULL)
                return 0;
            q->var = var_init(strdup(name), strdup(value));
            q->next = vars->data[h];
            vars->data[h] = q;
        }
        else
        {
            // key present
            // free(name);
            free(p->var->value);
            p->var->value = strdup(value);
        }
    }
    else
    {
        vars->data[h] = malloc(sizeof(struct pair_list));
        if (vars->data[h] == NULL)
            return 0;
        vars->data[h]->var = var_init(strdup(name), strdup(value));
        vars->data[h]->next = NULL;
    }
    return 1;
}

/* VARMAP GET */
char *varmap_get(struct varmap *vars, const char *varname)
{
    if (vars == NULL || vars->data == NULL)
        return NULL;
    for (size_t i = 0; i < vars->size; i++)
    {
        for (struct pair_list *p = vars->data[i]; p != NULL; p = p->next)
        {
            if (strcmp(p->var->name, varname) == 0)
                return p->var->value;
        }
    }
    char *res = getenv(varname);
    if (!res && vars_g && vars_g != vars)
        return varmap_get(vars_g, varname);
    return res;
}

/* VARMAP FIND */
static int varmap_find(struct varmap *vars, const char *varname)
{
    if (vars == NULL)
        return -1;
    for (size_t i = 0; i < vars->size; i++)
    {
        for (struct pair_list *p = vars->data[i]; p != NULL; p = p->next)
        {
            if (strcmp(p->var->name, varname) == 0)
                return i;
        }
    }
    return -1;
}

/* VARMAP REMOVE */
int varmap_remove(struct varmap *vars, char *varname)
{
    int i = varmap_find(vars, varname);
    if (i == -1)
        return 0;

    struct pair_list *p = vars->data[i];
    if (p->next == NULL)
    {
        vars->data[i] = NULL;
        pair_list_destroy(p);
        return 1;
    }

    if (strcmp(p->var->name, varname) == 0)
    {
        struct pair_list *tmp = p;
        p = p->next;
        pair_list_destroy(tmp);
        vars->data[i] = p;
        return 1;
    }

    while (p->next != NULL)
    {
        if (strcmp(p->next->var->name, varname) == 0)
        {
            struct pair_list *tmp = p->next;
            p->next = p->next->next;
            pair_list_destroy(tmp);
            return 1;
        }
        p = p->next;
    }
    return 0;
}

void varmap_print(struct varmap *vars)
{
    if (vars == NULL)
        return;
    printf("\n ======= VARMAP PRINT ====== \n");
    for (size_t i = 0; i < vars->size; i++)
    {
        if (vars->data[i] != NULL)
        {
            size_t count = 0;
            struct pair_list *p = vars->data[i];
            while (p != NULL)
            {
                p = p->next;
                count += 1;
            }

            p = vars->data[i];
            size_t i = 0;
            while (i < count)
            {
                if (i == 0)
                    printf("%s: %s", p->var->name, p->var->value);
                else
                    printf(", %s: %s", p->var->name, p->var->value);
                i += 1;
                p = p->next;
            }
            printf("\n");
        }
    }
    printf("\n ======= VARMAP PRINT ====== \n");
}

// =============================== EXPANSION ==================================

static int is_varname(char c)
{
    return isalpha(c) || isdigit(c) || c == '_';
}

static int is_special_var(char c)
{
    return isdigit(c) || c == '#' || c == '$' || c == '@' || c == '*'
        || c == '?';
}

static int is_valid_var_start(char c)
{
    return isalpha(c) || is_special_var(c) || c == '{';
}

static int is_var_to_replace(char c, char aft, char prev, int s_quoted)
{
    return c == '$' && prev != '\\' && !s_quoted && aft != '\0'
        && is_valid_var_start(aft) && aft != '(';
}

/* NEW VERSION THAT CHECKS SINGLE QUOTES */
int is_expandable(char *str)
{
    if (!str || str[0] == '\0')
        return 0;
    if (str[0] == '$' && is_valid_var_start(str[1]))
        return 1;
    char prev = str[0];
    int s_quoted = (str[0] == '\'') ? 1 : 0;
    for (size_t i = 1; str[i] != '\0'; i++)
    {
        if (is_var_to_replace(str[i], str[i + 1], prev, s_quoted))
            return 1;
        if (str[i] == '\'')
            s_quoted = !s_quoted;
        prev = str[i];
    }
    return 0;
}

/* RETURNS LENGTH OF VAR NAME + ACCESSORIES */
static char *skip_varname(char *str, size_t *ind_start, size_t *ind_end,
                          size_t *i)
{
    int curly = 0;
    *i += 1; // skipping $
    *ind_start = *i;
    if (str[*i] == '\0')
        return NULL;
    if (str[*i] == '{')
    {
        curly = 1;
        *i += 1;
        *ind_start += 1;
    }

    if (is_special_var(str[*i]))
    {
        *i += 1;
    }
    else
    {
        while (is_varname(str[*i]))
            *i += 1;
    }

    *ind_end = *i;
    *i += curly;

    size_t len = *ind_end + 1 + 2 * curly - *ind_start;
    if (len == 0)
        return NULL;
    return strndup(str + (*ind_start - 1 - curly), len);
}

char *replace_var(char *str, char *varname, char *value)
{
    char *result;
    int i = 0;
    int count = 0;
    int varname_len = strlen(varname);
    int value_len = strlen(value);

    // Counting the number of times old word
    // occur in the string
    for (i = 0; str[i] != '\0'; i++)
    {
        if (strstr(&str[i], varname) == &str[i])
        {
            count++;

            // Jumping to index after the old word.
            i += varname_len - 1;
        }
    }

    // Making new string of enough length
    result = calloc((i + count * (value_len - varname_len) + 1), sizeof(char));

    i = 0;
    while (*str)
    {
        // compare the substring with the result
        if (strstr(str, varname) == str)
        {
            strcpy(&result[i], value);
            i += value_len;
            str += varname_len;
        }
        else
            result[i++] = *str++;
    }

    // free(recipe); // ?
    result[i] = '\0';
    return result;
}

/* EXPAND VAR */
char *expand_var(struct varmap *vars, char *str)
{
    while (is_expandable(str))
    {
        size_t i = 0;
        if (str[i] != '$')
        {
            i += 1;
            while (str[i] != '\0' && str[i] != '$' && str[i - 1] != '\\')
                i += 1;
            if (str[i] == '\0')
                break;
        }
        size_t start = 0;
        size_t end = 0;
        char *varname = skip_varname(str, &start, &end, &i);

        if (!varname)
            break;

        char *name = strndup(str + start, end - start);
        add_pid(vars);

        char *value = varmap_get(vars, name);
        if (!value && vars != vars_g) // vars is not the main one
            value = varmap_get(vars_g, name);
        free(name);

        char *tmp = str;
        if (!value) // var doesnt exist
            str = replace_var(str, varname, "");
        else
            str = replace_var(str, varname, value);
        free(tmp);
        free(varname);
    }
    return str;
}
