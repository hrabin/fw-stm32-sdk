#ifndef UTIL_H
#define UTIL_H

#include "common.h"

long lsqrt(uint32_t a);
char strnicmp(const char *str1, const char *str2, unsigned char len);
char stricmp(const char *str1, const char *str2);
char stricmp2(const char *str1, const char *str2);
u8 ascii_to_hex(ascii ch);
bool is_char(char ch);
int is_number(const char *str, int limit);
int hex_to_bin(u8 *dest, const char *src, int limit);

#endif // ! UTIL_H

