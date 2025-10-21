#include "parse.h"
#include "common.h"

static void _skip_spaces(char **pptext)
{
    while (**pptext == ' ')
    {
        (*pptext)++;
    }
}

char *parse_pattern(const char *s, const char *pattern)
{
    char *p = (char *)s;

    int len = strlen(pattern);

    if (strncmp(p, pattern, len) != 0)
    {
        return NULL;
    }

    return p + len;
}

char *parse_separator(const char *s)
{
    char *p = (char *)s;

    _skip_spaces(&p);

    if (*p++ != ',')
    {
        return NULL;
    }

    return p;
}

char *parse_terminator(const char *s)
{
    char *p = (char *)s;

    _skip_spaces(&p);

    if (*p != '\0')
    {
        return NULL;
    }

    return p;
}

char *parse_string(char *dest, const char *s, int size)
{
    char *p = (char *)s;

    _skip_spaces(&p);

    if (*p++ != '"')
        return(NULL);

    p = strchr(p, '"');

    if (p == NULL)
        return(NULL);

    if (dest != NULL)
    {
        int len = p - s - 1;

        if (len >= size)
            return(NULL);

        memset(dest, 0, size);
        strncpy(dest, s + 1, len);
    }

    return (p + 1);
}

#if defined PRIu64
char *parse_u64(u64 *dest, const char *s)
{
    char *p = (char *)s;
    int n;

    if (sscanf(s, "%" PRIu64 "%n", dest, &n) != 1)
        return (NULL);

    return (p+n);
}
#else
  #warning "print without 64bit support"
#endif // defined PRIu64

inline static bool _isdigit(char ch)
{
    int a = ch;
    return (isdigit(a) ? true : false);
}

char *parse_number(parse_number_t *number, const char *s)
{
    char *p = (char *) s;
    parse_number_t num;
    int n;

    if (sscanf(s, "%" PRId32 "%n", &num, &n) != 1)
        return (NULL);

    if (number != NULL)
    {
        *number = num;
    }
    return (p + n);
}

char *parse_modem_separator(const char *s)
{
    char *p = (char *) s;

    _skip_spaces(&p);

    if (*p++ != ',')
        return (NULL);

    return p;
}
