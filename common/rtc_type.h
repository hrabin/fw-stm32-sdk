#ifndef RTC_TYPE_H
#define RTC_TYPE_H

#include "type.h"

// Pure time-stamp data type — no hardware dependency.
// Used by drivers (drv_g4/rtc.h), libraries (lib/nmea), and application code.

typedef struct {
	u8 second;
	u8 minute;
	u8 hour;
	u8 day;
	u8 month;
	u8 year;   // YEAR = year + 2000
} __attribute__((packed)) rtc_t;

#endif // ! RTC_TYPE_H
