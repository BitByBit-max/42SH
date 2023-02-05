#ifndef LOG_H
#define LOG_H

#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../token/token.h"

FILE *set_up_log(char *log_file);
void log_text(char *s);
void log_int(char *s, int i);
void log_char(char *s, char c);
void log_2int(char *s1, int i1, char *s2, int i2);
void log_token(int type, char *data);
void close_file(void);

#endif /* ! LOG_H */
