#ifndef SPECIAL_VARS_H
#define SPECIAL_VARS_H

#include "../varmap/varmap.h"

char *my_itoa(int value);
void add_pid(struct varmap *vars);
void add_variables(struct varmap *vars, int argc, char *argv[]);
void add_variables_func(struct varmap *vars, int argc, char *argv[],
                        char *name);

#endif
