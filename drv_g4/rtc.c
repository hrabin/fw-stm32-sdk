#include "common.h"
#include "rtc.h"
#include "log.h"
#include "irq.h"

#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_rtc.h"
#include "stm32g4xx_ll_pwr.h"

LOG_DEF("RTC");

const u8 MONTH_DAYS[13] = {0,31,29,31,30,31,30,31,31,30,31,30,31};

const rtc_t RTC_DEF = {
    .second = 0,
    .minute = 0,
    .hour   = 8,
    .day    = 1,
    .month  = 1,
    .year   = 24};



void RTC_WKUP_IRQHandler(void)
{
    OS_PRINTF("Wakeup IRQ\n");
    LL_RTC_ClearFlag_WUT(RTC);
}

bool rtc_init(void)
{
    LL_PWR_EnableBkUpAccess();

    LL_RCC_EnableRTC();
  
    LL_RTC_DisableWriteProtection(RTC);
    LL_RTC_EnableInitMode(RTC);
    
    LL_RTC_SetHourFormat(RTC, LL_RTC_HOURFORMAT_24HOUR);
    LL_RTC_SetSynchPrescaler(RTC, 255);
    LL_RTC_SetAsynchPrescaler(RTC, 127);

    LL_RTC_EnableShadowRegBypass(RTC);
    
    // Exit initialization mode
    LL_RTC_DisableInitMode(RTC);

    // Enable write protection for RTC
    LL_RTC_EnableWriteProtection(RTC);
    LL_PWR_DisableBkUpAccess();

    // HAL_RTCEx_SetWakeUpTimer_IT
    return (true);
}

bool rtc_valid(const rtc_t *t)
{
    u8 mdays;

    if ((t->year<21) || (t->year>100)
     || (t->month<1) || (t->month>12)
     || (t->day<1) || (t->day>31)
     || (t->hour>23) || (t->minute>59) || (t->second>59))
        return (false);

    mdays = MONTH_DAYS[t->month];
    if (((t->year & 0x03) != 0) && (t->month == 2))
        mdays--;

    if (t->day>mdays)
        return (false);

    return (true);
}

void rtc_get_time (rtc_t *t)
{
    t->hour   = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetHour(RTC));
    t->minute = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetMinute(RTC));
    t->second = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetSecond(RTC));

    t->day   = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetDay(RTC));
    t->month = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetMonth(RTC));
    t->year  = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetYear(RTC));
}

void rtc_set_time (const rtc_t *t)
{
    if (! rtc_valid(t))
        return;

    LOG_DEBUG("set time");
    LL_RTC_DisableWriteProtection(RTC);
    LL_RTC_EnableInitMode(RTC);

    while (! LL_RTC_IsActiveFlag_INIT(RTC))
        ; // Wait until initialization mode is ready

    // Set date
    LL_RTC_DATE_SetDay(RTC, __LL_RTC_CONVERT_BIN2BCD(t->day));
    LL_RTC_DATE_SetMonth(RTC, __LL_RTC_CONVERT_BIN2BCD(t->month));
    LL_RTC_DATE_SetYear(RTC, __LL_RTC_CONVERT_BIN2BCD(t->year));
    
    // Set time (24-hour format)
    LL_RTC_TIME_SetHour(RTC, __LL_RTC_CONVERT_BIN2BCD(t->hour));
    LL_RTC_TIME_SetMinute(RTC, __LL_RTC_CONVERT_BIN2BCD(t->minute));
    LL_RTC_TIME_SetSecond(RTC, __LL_RTC_CONVERT_BIN2BCD(t->second));

    // Exit initialization mode
    LL_RTC_DisableInitMode(RTC);
    LL_RTC_EnableWriteProtection(RTC);

}

void rtc_task(void)
{
}

void rtc_sub_hours (rtc_t *t, int h)
{
    u8 mdays;
    
    if ((h>23) || (h<-23) || (h==0))
        return;

    if (h<0)
    {
        rtc_add_hours (t, 0-h);
        return;
    }

    if (rtc_valid(t) == false)
        return;

    if (t->hour >= h)
    {
        t->hour -= h;
        return;
    }
    // we are going a day backward
    t->hour = t->hour + 24 - h;

    if (--(t->day) > 0)
        return;

    if (--(t->month) == 0)
    {
        t->month=12;
        t->year--;
    }

    mdays = MONTH_DAYS[t->month];
    if (((t->year & 0x03) != 0) && (t->month == 2))
        mdays--;
    t->day = mdays;
}

void rtc_add_hours (rtc_t *t, int h)
{
    u8 mdays;

    if ((h>23) || (h<-23) || (h==0))
        return; 

    if (h<0)
    {
        rtc_sub_hours (t, 0-h);
        return;
    }

    if (rtc_valid(t) == false)
        return;

    t->hour += h;
    if (t->hour < 24)
        return;

    t->hour -= 24;

    mdays = MONTH_DAYS[t->month];
    if (((t->year & 0x03) != 0) && (t->month == 2))
        mdays--;

    if (++(t->day) <= mdays)
        return;
    t->day = 1;
    if (++(t->month) <= 12)
        return;
    t->month=1;
    t->year++;
}


static int _day_of_week(int year, int month, int day)
{
    const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    year -= month < 3;
    return (year + year/4 - year/100 + year/400 + t[month-1] + day) % 7;
}

// Function to find the last Sunday of a given month in a given year
static int _get_last_sunday(int year, int month) 
{
    int last_day = 31; // March and October have 31 days
    int dow = _day_of_week(year, month, last_day);  // Get the day of the week for the last day of the month
    return last_day - dow;  // Subtract the day of the week to find the last Sunday
}

// Function to check if DST is active
bool rtc_dst_active(rtc_t *rtc) 
{
    int year = rtc->year + 2000;
    int last_sunday_march;
    int last_sunday_october; 

    // If between April and September, DST is active
    if ((rtc->month > 3) && (rtc->month < 10))
        return (true);

    // If in March, DST is active after the last Sunday
    if (rtc->month == 3)
    {
        last_sunday_march = _get_last_sunday(year, 3);
        if (rtc->day >= last_sunday_march)
            return (true);
    }
    // If in October, DST is active before the last Sunday
    if (rtc->month == 10)
    {
        last_sunday_october = _get_last_sunday(year, 10);
        if (rtc->day < last_sunday_october)
            return (true);
    }

    // Otherwise, DST is not active
    return (false);
}

