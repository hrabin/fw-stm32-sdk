#ifndef RTC_H
#define RTC_H

#include "type.h"

typedef struct {
    u8 second;
    u8 minute;
    u8 hour;
    u8 day;
    u8 month;
    u8 year;   // YEAR=year+2000
} __attribute__((packed)) rtc_t;

bool rtc_init(void);
bool rtc_valid(const rtc_t *t);
void rtc_get_time(rtc_t *time);
void rtc_set_time(const rtc_t *time);
void rtc_add_hours(rtc_t *t, int h);
bool rtc_dst_active(rtc_t *rtc);

#endif // ! RTC_H
