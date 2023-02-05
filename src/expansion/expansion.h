#ifndef EXPANSION_H
#define EXPANSION_H

#include "../varmap/varmap.h"

/* Removes quotes which are open and then closed while preserving the text
 * inside them */
char *expand_str(char *to_expand, int remove_quotations);

#endif /* ! EXPANSION_H */
