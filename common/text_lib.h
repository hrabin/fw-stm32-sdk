#ifndef 	TEXT_LIB_H
#define		TEXT_LIB_H

#include "type.h"

bool   text_ip_parse (u32 *ip, u16 *port, const ascii *src);
u16    text_sms_detox (ascii * out_text, ascii * sms_text, u16 len);
ascii *text_limit (ascii * text, u16 len);
u8     text_ucs2_char_to_utf8 (ascii *dest, u16 ucs2_char);
u16    text_ucs2_to_utf8 (ascii * dest, ascii * src, u16 limit_len);
u16    text_utf8_to_ascii (ascii *data, u16 limit_len);
u16    text_get_ucs2_char_from_utf8 (ascii *src_ptr, u8 *readed_chars);
u16    text_utf8_to_ucs2 (ascii * dest, ascii * src, u16 *in_bytes, u16 limit_len);
bool   text_is_ascii (ascii * text);
bool   text_is_valid_phone (const ascii *phone);
bool   text_phone_compare (ascii * text1, ascii * text2);
u16    text_utf_oso_to_utf8 (ascii * dest, ascii * src, u16 limit_len);
s16    text_get_index (u32 text_num);

#endif // TEXT_LIB_H
