#ifndef PARSE_H
#define PARSE_H

#include "type.h"

typedef s32 parse_number_t;

char *parse_pattern(const char *s, const char *pattern);
char *parse_separator(const char *s);
char *parse_terminator(const char *s);
char *parse_string(char *buffer, const char *s, int limit);
char *parse_u64(u64 *dest, const char *s);
char *parse_number(parse_number_t *number, const char *s);
char *parse_modem_separator(const char *s);

#endif // PARSE_H
