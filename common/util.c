#include "util.h"

long lsqrt(uint32_t a)
{   // get root of 'a'
    long x_old, x_new;
    long test;
    int nbits;
    int i;

    // select a good starting value using binary logarithms: 
    nbits = (sizeof(a) * 8) - 1; // subtract 1 for sign bit 

    for (i = 4, test = 16L; ; i += 2, test <<= 2L)
    {
        if (i >= nbits || a <= test)
        {
            x_old = (1L << (i / 2L)); // x_old = sqrt(test) 
            break;
        }
    }
    // use the Babylonian method to arrive at the integer square root: 
    for (;;) 
    {
        x_new = (a / x_old + x_old) / 2L;
        if (x_old <= x_new)
            break;
        x_old = x_new;
    }

    return x_old;
}

inline static bool _charicmp(const char a, const char b)
{
	if (a == b)
		return (true);
	if (((a >= 'A') && (a <= 'Z')) && (a + ('a' - 'A') == b))
		return (true);
	if (((b >= 'A') && (b <= 'Z')) && (b + ('a' - 'A') == a))
		return (true);

	return (false);
}

char strnicmp(const char *str1, const char *str2, unsigned char len)
{
	char a, b;

	for (; len; len--)
	{
		a = *(str1++);
		b = *(str2++);
		
		if ((a=='\0') && (b=='\0'))
			break; // oba retezce zde konci
		
		if (_charicmp(a, b))
			continue;
		
		return (a - b);

	}
	return (0);
}

char stricmp(const char *str1, const char *str2)
{
	char a, b;
	u16 len = 1024;

	for (; len; len--)
	{
		a = *(str1++);
		b = *(str2++);
		if ((a == '\0') && (b == '\0'))
			return (0);	// oba retezce zde konci

		if (_charicmp(a, b))
			continue;
		return (a - b);

	}
	return (64);
}

char stricmp2(const char *str1, const char *str2)
{	// use length from second parameter 
	char a, b;
	u16 len = 1024;

	for (; len; len--)
	{
		a = *(str1++);
		b = *(str2++);
		if (b == '\0')
			return (0);	// druhy retezec zde konci

		if (_charicmp(a, b))
			continue;
		return (a - b);

	}
	return (64);
}

u8 ascii_to_hex(ascii ch)
{
	if ((ch >= '0') && (ch <= '9'))
		return (ch - '0');
	if ((ch >= 'a') && (ch <= 'f'))
		return (ch - 'a' + 10);
	if ((ch >= 'A') && (ch <= 'F'))
		return (ch - 'A' + 10);
	return (0);
}

bool is_char(char ch)
{
    if ((ch >= 'a') && (ch <= 'z'))
        return (true);
    if ((ch >= 'A') && (ch <= 'Z'))
        return (true);
    if ((ch >= '0') && (ch <= '9'))
        return (true);
    if ((ch == '*') || (ch == '#'))
        return (true);

    return (false);
}

int is_number(const char *str, int limit)
{
    int i;

    for (i = 0; (i < limit) && (*str != '\0'); i++, str++)
    {
        if ((*str < '0') || (*str > '9'))
        {
            return (0);
        }
    }
    return (i);
}

int hex_to_bin(u8 *dest, const char *src, int limit)
{
	int i;

	for (i=0; i<limit; i++)
	{
		unsigned int ch;
		if (sscanf(src, "%02x", &ch) != 1)
			break;;
		*dest++ = ch;
		src += 2;
	}
	return (i);
}
