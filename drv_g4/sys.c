#include "common.h"
#include "sys.h"
#include "reset.h"
#include "gpreg.h"

#include <stm32g4xx_ll_rcc.h>
#include <stm32g4xx_ll_system.h>
#include <stm32g4xx_ll_cortex.h>
#include <stm32g4xx_ll_utils.h>
#include <stm32g4xx_ll_pwr.h>

#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007)
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006)
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005)
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004)
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003)

static u8 reset_type = RESET_POWER_ON;

void sys_init(void)
{
    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN); // __HAL_RCC_SYSCFG_CLK_ENABLE()
    SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_PWREN);  // __HAL_RCC_PWR_CLK_ENABLE()
    SET_BIT(PWR->CR3, PWR_CR3_UCPD_DBDIS);       // HAL_PWREx_DisableUCPDDeadBattery()

    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    // System interrupt init
    // PendSV_IRQn interrupt configuration
    NVIC_SetPriority(PendSV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),15, 0));
}

void sys_clock_config(void)
{
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
    while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1)
        ;
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
    LL_RCC_HSI_Enable();
    // Wait till HSI is ready
    while (LL_RCC_HSI_IsReady() != 1)
        ;
    LL_RCC_HSI_SetCalibTrimming(64);
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_1, 12, LL_RCC_PLLR_DIV_4);
    LL_RCC_PLL_EnableDomain_SYS();
    LL_RCC_PLL_Enable();
    // Wait till PLL is ready
    while(LL_RCC_PLL_IsReady() != 1)
        ;
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    // Wait till System clock is ready
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
        ;
    // Set AHB prescaler
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_SetSystemCoreClock(SYSCLK);
    LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK2);
    LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1);
    LL_RCC_SetUSARTClockSource(LL_RCC_USART3_CLKSOURCE_PCLK1);
    LL_RCC_SetUARTClockSource(LL_RCC_UART4_CLKSOURCE_PCLK1);
    
    LL_PWR_EnableBkUpAccess();
    LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_LOW);
    LL_RCC_LSE_Enable();
    // Wait till LSE is ready
    while (LL_RCC_LSE_IsReady() != 1)
        ;

    LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
    LL_PWR_DisableBkUpAccess();

}

void sys_run(void)
{   
    u32 reset_source = (RCC->CSR & 0xFE000000UL);

    RCC->CSR |= RCC_CSR_RMVF;

    // reset_type default je RESET_POWER_ON
   
    if (reset_source & RCC_CSR_LPWRRSTF)
    {   // Low-power reset flag
        reset_type=RESET_BOD;
    }   
    else if (GPREG_WDID == GPREG_WDID_REBOOT_RQ)
    {
        reset_type=RESET_USER_RQ;
        // dont clear flag, let it for app
        // GPREG_WRITE(GPREG_WDID, 0);
    }
    else if (reset_source & RCC_CSR_IWDGRSTF)
    {   // Independent window watchdog reset flag
        reset_type=RESET_WDT;
    }
    else if (reset_source & RCC_CSR_WWDGRSTF)
    {   // Window watchdog reset flag
        reset_type=RESET_WDT;
    }
    else if (reset_source & RCC_CSR_SFTRSTF)
    {   // Software reset flag
        reset_type=RESET_WDT;
    }
    else if (reset_source & RCC_CSR_PINRSTF) 
    {   // Pin reset flag (from the NRST pin)
        reset_type=RESET_POWER_ON;
    }
}

u8 sys_get_reset(void)
{
    return (reset_type);
}
