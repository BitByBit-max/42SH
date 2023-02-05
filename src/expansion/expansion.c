#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../string_t/string_t.h"

static struct string *new_str;

static int handle_double_quote(char *to_expand, int starting_index,
                               int remove_quotations)
{
    char current_char;
    int i = starting_index + 1;
    for (; (current_char = to_expand[i]) != '\0'; i++)
    {
        if (current_char == '\\')
        {
            char next_char = to_expand[i + 1];
            if (next_char == '`' || next_char == '"' || next_char == '\\'
                || next_char == '\n' || next_char == '$')
            {
                new_str = string_append(new_str, next_char);
                i++;
                if (to_expand[i] == '\0')
                    break;
                continue;
            }
            new_str = string_append(new_str, current_char);
        }
        else
        {
            if (current_char == '\"' && remove_quotations)
            {
                break;
            }
            new_str = string_append(new_str, current_char);
        }
    }
    return i;
}

/* returns the index of the last single quote */
static int handle_single_quote(char *to_expand, int starting_index,
                               int remove_quotations)
{
    char current_char;
    int i = starting_index + 1;
    for (; (current_char = to_expand[i]) != '\0'; i++)
    {
        if (current_char == '\'' && remove_quotations)
            break;
        new_str = string_append(new_str, current_char);
    }
    return i;
}

char *expand_str(char *to_expand, int remove_quotations)
{
    new_str = string_init(16);

    char current_char;
    for (int i = 0; (current_char = to_expand[i]) != '\0'; i++)
    {
        if (current_char == '\\') // unquoted backslash
        {
            i++;
            current_char = to_expand[i];
            new_str = string_append(new_str, current_char);
            if (to_expand[i] == '\0')
                break;
            continue;
        }
        else if (current_char == '\'') // if has single quote
        {
            if (!remove_quotations)
                new_str = string_append(new_str, current_char);
            i = handle_single_quote(to_expand, i, remove_quotations);
            if (to_expand[i] == '\0')
                break;
            continue;
        }
        else if (current_char == '\"')
        {
            if (!remove_quotations)
                new_str = string_append(new_str, current_char);
            i = handle_double_quote(to_expand, i, remove_quotations);
            if (to_expand[i] == '\0')
                break;
            continue;
        }
        new_str = string_append(new_str, current_char);
    }
    if (new_str->size != 0)
        new_str = string_append(new_str, '\0');
    char *res = new_str->data;
    string_destroy(new_str);
    free(to_expand);
    return res;
}
