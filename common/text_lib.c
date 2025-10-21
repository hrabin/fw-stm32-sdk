#include "os.h"
#include "const.h"
#include "text_lib.h"
#include "util.h"

#include <string.h>
#include <stdio.h>

#define SYMB_SPACE ' '

typedef struct {
    u8    src[5];
    ascii dest;
} sms_utf8_to_ascii_table_t;

const sms_utf8_to_ascii_table_t UTF8_TO_ASCII_TABLE[] = {
{"à",'a'},{"á",'a'},{"â",'a'},{"ã",'a'},{"ä",'a'},{"å",'a'},{"ą",'a'},
{"Á",'A'},{"Â",'A'},{"Ã",'A'},{"Ä",'A'},{"Å",'A'},{"Ą",'A'},
{"č",'c'},{"ć",'c'},{"ĉ",'c'},{"ċ",'c'},
{"Č",'C'},{"Ć",'C'},{"Ĉ",'C'},{"Ċ",'C'},
{"ď",'d'},
{"Ď",'D'},{"Ð",'D'},
{"ě",'e'},{"é",'e'},{"è",'e'},{"ę",'e'},
{"È",'E'},{"É",'E'},{"Ê",'E'},{"Ë",'E'},{"Ě",'E'},
{"í",'i'},
{"Ì",'I'},{"Í",'I'},{"Î",'I'},{"Ï",'I'},
{"ĺ",'l'},{"ļ",'l'},{"ľ",'l'},{"ł",'l'},
{"Ĺ",'L'},{"Ļ",'L'},{"Ľ",'L'},{"Ŀ",'L'},
{"ň",'n'},{"ń",'n'},
{"Ň",'N'},{"Ñ",'N'},{"Ń",'N'},
{"ó",'o'},{"ō",'o'},{"ŏ",'o'},{"ő",'o'},
{"Ó",'O'},{"Ò",'O'},{"Ô",'O'},{"Õ",'O'},{"Ö",'O'},
{"ř",'r'},{"ŕ",'r'},
{"Ř",'R'},{"Ŕ",'R'},
{"š",'s'},{"ś",'s'},
{"Š",'S'},{"Ś",'S'},
{"ť",'t'},
{"Ť",'T'},
{"ú",'u'},{"ů",'u'},{"ũ",'u'},{"ū",'u'},{"ŭ",'u'},{"ű",'u'},{"ų",'u'},
{"Ú",'U'},{"Ů",'U'},{"Ũ",'U'},{"Ū",'U'},{"Ŭ",'U'},{"Ű",'U'},{"Ų",'U'},{"Ù",'U'},{"Û",'U'},{"Ü",'U'},
{"ý",'y'},{"ỳ",'y'},{"ỹ",'y'},
{"Ý",'Y'},{"Ÿ",'Y'},{"Ỳ",'Y'},{"Ỹ",'Y'},
{"ž",'z'},
{"Ž",'Z'},
{"",'\0'}
};

s16 text_find(const ascii * long_text, const ascii * short_text)
{
    u16 i, j;
    u16 len;

    len = strlen(long_text);


    for (i = 0; i < len; i++)
    {
        j = 0;
        while (*(short_text + j) == *(long_text + j))
        {
            j++;
            if (*(short_text + j) == '\0')
                return (i);
        }
        if (*long_text == '\0')
            return (-1);
        long_text++;
    }
    return (-1);
}

ascii *text_limit(ascii * text, u16 len)
{
    if (strlen(text) > len)
    {
        *(text + len) = '\0';
        if (len > 3)
        {
            *(text + len - 1) = '.';
            *(text + len - 2) = '.';
            *(text + len - 3) = '.';
        }
    }
    return (text);
}

u8 text_ucs2_char_to_utf8 (ascii *dest, u16 ucs2_char)
{ 
    u8 written = 1;

    if ((ucs2_char == 0) || (ucs2_char == 0xFFFF))
    {
        *dest = '\0';
    }
    else if (ucs2_char < 0x80)
    {
        *dest = ucs2_char & 0x7F;
    }
    else if (ucs2_char < 0x800)
    {
        *(dest++) = 0xC0 + (ucs2_char >> 6);
        *(dest++) = 0x80 + (ucs2_char & 0x3F);
        written = 2;
    }
    else
    {
        *(dest++) = 0xE0 + (ucs2_char >> 12);
        *(dest++) = 0x80 + ((ucs2_char >> 6) & 0x3F);
        *(dest++) = 0x80 + (ucs2_char & 0x3F);
        written = 3;
    }
    return (written);
}

u16 text_ucs2_to_utf8(ascii * dest, ascii * src, u16 limit_len)
{   
    u16 i;
    u16 ucs2_char;
    u8 *src_ptr;
    u8 tmp;

    src_ptr = (u8 *)src;

    for (i = 0; i < (limit_len - 3); )
    {
        // UCS is BIG endian
        ucs2_char  = *(src_ptr++)<<8;
        ucs2_char += *(src_ptr++);
        tmp = text_ucs2_char_to_utf8 (dest, ucs2_char);
        if ((*dest == '\0')
         || (tmp==0))
            break;
        i+=tmp;
        dest += tmp;
    }
    *dest = '\0';
    return (i);
}

u16 text_utf8_to_ascii (ascii *data, u16 limit_len)
{   // replace some UTF8 character by similar matching ASCII (see UTF8_TO_ASCII_TABLE)
    ascii *src_ptr;
    u8 ch;
    u16 i;
    u16 out_len=0;
    u16 len;
    u8 t, char_len; // number of bytes for one UTF8 character
    
    len=strlen(data);
    if (len>limit_len)
        len=limit_len;

    src_ptr = data;
    for (i=0; i<len; i++)
    {   
        ch=*src_ptr;
        data[out_len]=ch;
        if (ch=='\0')
            break;
        if ((ch & 0x80) == 0)
        {   // standard ASCII, copy it
            out_len++;
            src_ptr++;
            continue;
        }
        else if ((ch & 0xE0) == 0xC0)
        {   // 2 chars
            char_len=2;
        }
        else if ((ch & 0xF0) == 0xE0)
        {   // 3 chars
            char_len=3;
        }
        else
        {   // unsupported character, just skip
            data[out_len]='?';
            out_len++;
            src_ptr++;
            continue;
        }
        for (t=0; t<200; t++)
        {
            if (UTF8_TO_ASCII_TABLE[t].dest == '\0')
            {   // end of table, cant convert
                data[out_len]='?';
                src_ptr+=char_len;
                break;
            }
            if (memcmp(UTF8_TO_ASCII_TABLE[t].src, src_ptr, char_len) == 0)
            {   // souhlasi, mam znak na konverzi
                if (UTF8_TO_ASCII_TABLE[t].src[char_len] == 0)
                {
                    src_ptr+=char_len;
                    data[out_len]=UTF8_TO_ASCII_TABLE[t].dest;
                    break;
                }
            }
        }
        out_len++;
    }
    return (out_len);
}

u16 text_get_ucs2_char_from_utf8 (ascii *src, u8 *readed_chars)
{ 
//    0000 0000-0000 007F | 0xxxxxxx
//    0000 0080-0000 07FF | 110xxxxx 10xxxxxx
//    0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
// X  0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
// UTF8 0xc4 0x9b == 110[00100] 10[011011] --> [00100][011011] --> UCS2 0x011B

    u16 ucs2_char;
    u8 readed;

    if (((*src) & 0x80) == 0)
    {   // 0xxxxxxx
        ucs2_char = *(src++);
        readed = 1;
    }
    else if (((*src) & 0xE0) == 0xC0)
    {   // 110xxxxx 10xxxxxx
        ucs2_char = ((*(src++)) & 0x1F);
        ucs2_char <<= 6;
        ucs2_char += ((*(src)++) & 0x3F);
        readed = 2;
    }
    else if (((*src) & 0xF0) == 0xE0)
    {   // 1110xxxx 10xxxxxx 10xxxxxx
        ucs2_char = ((*(src++)) & 0x0F);
        ucs2_char <<= 6;
        ucs2_char = ((*(src++)) & 0x3F);
        ucs2_char <<= 6;
        ucs2_char += ((*(src++)) & 0x3F);
        readed = 3;
    }
    else
    {   // cant convert
        src++;
        ucs2_char = '?';
        readed = 1;
    }

    if (readed_chars != NULL)
        *readed_chars = readed;

    return (ucs2_char);
} 

u16 text_utf8_to_ucs2 (ascii *dest, ascii *src, u16 *in_bytes, u16 limit_len)
{   // in_bytes return number of processed input bytes
    u16 i;
    u16 out_len = 0; // return length in bytes
    u16 ucs2_char;
    u16 len;
    ascii *src_ptr;
    u8 tmp;
    
    src_ptr=src;
    len = strlen(src_ptr) & 0x1FF;

    for (i = 0; i < len; )
    {
        ucs2_char = text_get_ucs2_char_from_utf8 (src_ptr, &tmp);
        src_ptr += tmp;
        i += tmp;

        if (dest != NULL)
        {   // if dest==NULL, then only calculate length
            *(dest++) = (ucs2_char >> 8) & 0xFF;
            *(dest++) = ucs2_char & 0xFF;
        }
        out_len++;

        if (limit_len)
        {
            if (out_len >= limit_len)
                break;
        }

    }
    if (in_bytes != NULL)
        *in_bytes = src_ptr-src;
    return (out_len);
}   

bool text_phone_compare(ascii * text1, ascii * text2)
{   // compare phone number from the end 
    u16 i;
    u16 num_chars = 0;
    u16 len1, len2;

    len1 = strlen(text1);
    len2 = strlen(text2);

    if (len1 == 0)
        return (false);
    if (len2 == 0)
        return (false);

    for (i = len1; i > 0; i--)
    {
        len1--;
        len2--;
        num_chars++;
        if (*(text1 + len1) != *(text2 + len2))
            return (false);
        if ((len2 == 0) || (num_chars > 8)) // 9 characters is enough
            return (true); 
    }
    return (true); // OK, whole length of number match
}


bool text_is_valid_phone (const ascii *phone)
{ 
    ascii tmp;
    u16 i;
    u16 len;

    if (phone == NULL)
        return (false);

    len = strnlen(phone, MAX_PHONENUM_LEN+1);

    if ((len == 0) 
     || (len>MAX_PHONENUM_LEN))
        return (false);

    for (i = 0; i < len; i++)
    {
        tmp = *(phone++);
        if ((tmp >= '0') && (tmp <= '9'))
            continue;
        if ((tmp == '+') && (i == 0))
            continue; // '+' may be only the first character

        return (false);
    }
    return (true);
} 

bool text_is_ascii(ascii * text)
{
    u16 len = strlen(text);

    for (; len; len--)
    {
        if (*(text++) & 0x80)
            return (false);
    }
    return (true);
}

