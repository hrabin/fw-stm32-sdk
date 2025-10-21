#ifndef  PLATFORM_SETUP_H
#define  PLATFORM_SETUP_H

#include "type.h"
#include "stm32g4xx.h"
#include "util.h"
#include "hardware.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PACK __attribute__((packed))
#define PACK_BEGIN
#define PACK_END

#define PLATFORM_NAME        "STM32xx"

#define NL "\r\n"

#define STRINGYFY(s) #s
#define FILELINE(line) __FILE__ ":" STRINGYFY(line)
#define ASSERT(p,msg) if(!(p)){ printf("ASSERT: " FILELINE(__LINE__) " " msg NL); while(1);}

#define RU16(data)          (*(data)+(*((data)+1)<<8))

#define ABS(x) (((x)<0) ? 0-(x) : (x))

#ifndef NULL
    #define NULL ((void *)0)
#endif

#define SYSCLK                  48000000
#define HCLK                    SYSCLK
#define PCLK1                   (HCLK/1)
#define APB1CLK                 (PCLK1/1)
#define TIM2CLK                 APB1CLK

// 0 == highest priority, 15 == lowest
#define IRQ_DEF_PRIO            7
#define IRQ_PRIO_ADC            5
#define IRQ_PRIO_RTC            3
#define IRQ_PRIO_UART1          IRQ_DEF_PRIO
#define IRQ_PRIO_UART2          IRQ_DEF_PRIO
#define IRQ_PRIO_UART3          IRQ_DEF_PRIO
#define IRQ_PRIO_UART4          8

#define UART1CLK                PCLK1
#define UART2CLK                PCLK1
#define UART3CLK                PCLK1
#define UART4CLK                PCLK1

#endif //  PLATFORM_SETUP_H
