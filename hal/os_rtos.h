#ifndef OS_RTOS_H
#define OS_RTOS_H

#include "platform_setup.h"
#include "cmsis_os.h"
#include "tty.h"

#include <stdio.h>
#include <stdlib.h>

#define _QUOTEME(x) #x
#define QUOTEME(x) _QUOTEME(x)

#define OS_ASSERT               ASSERT
#define OS_GETCHAR()            getChar()
#define OS_PLATFORM_NAME        PLATFORM_NAME

#define OS_MEM_ALLOC malloc
#define OS_MEM_FREE(name)     {free(name); name=NULL;}

#define OS_PUTTEXT(x)   printf(x) // console_put_text(x)
#define OS_PRINTF       printf
#define OS_VPRINTF      vprintf
#define OS_ERROR(...)   {printf("ERROR:" __VA_ARGS__); printf(NL); }
#define OS_WARNING(...) {printf("WARNING:" __VA_ARGS__); printf(NL); }
#define OS_FATAL( ... ) {printf("FATAL ERROR: " __VA_ARGS__); printf("\r\n"); while(1);}
#define OS_FLUSH() fflush(stdout)

void os_critical_exit(void);
void os_critical_enter(void);

#define OS_DELAY_US(x) time_delay_us(x)

#define TIMER_MS 1
#define OS_TIMER(x) os_timer_get(x)
#define OS_TIMER_MS (TIMER_MS)
#define OS_TIMER_SECOND (1000*TIMER_MS)
#define OS_TIMER_MINUTE (60*OS_TIMER_SECOND)

typedef u64 os_timer_t;
os_timer_t os_timer_get(void);

#define OS_DELAY(x) osDelay(x)

#define MS     OS_TIMER_MS
#define SECOND OS_TIMER_SECOND

#endif // ! OS_RTOS_H
