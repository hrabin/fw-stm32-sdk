#include "platform_setup.h"
#include "hardware.h"
#include "time.h"

#if TIMER2_ON 

void timer2_free_run(void)
{
	// enable clock
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
	// reset
	RCC->APB1RSTR1 |= RCC_APB1RSTR1_TIM2RST;
	RCC->APB1RSTR1 &= ~RCC_APB1RSTR1_TIM2RST;

	// Set the Autoreload value
	TIM2->ARR = (unsigned long)(0-1); //free run timer
 
	// Set the Prescaler value
	TIM2->PSC = (TIM2CLK/1000000)-1;
	
	// Generate an update event to reload the Prescaler value immediatly
	TIM2->EGR = TIM_EGR_UG;    //1 

	
	TIM2->CR1 = TIM_CR1_CEN; // Counter enable
}

void timer2_stop(void)
{
	// enable clock
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
	TIM2->CR1 = 0;
	// disable clock
    // RCC->APB1ENR &= ~RCC_APB1ENR_TIM2EN;
}

void timer2_run(void)
{
	// enable clock
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
	TIM2->CR1 = TIM_CR1_CEN; // Counter enable
}
#endif // TIMER2_ON 

void timer_delay_ms(u32 delay)
{
	utime_t time;
	
	delay*=TIMER_MS;
	time=timer_get_time();
	
	while((utime_t)(timer_get_time()-time)<delay)
	{

	}
}

void timer_delay_us (utime_t tm)
{	// 
	tm /= TIMER_US;

	if (tm>=1000)
	{
		OS_DELAY(tm/1000);
		tm %= 1000;
	}

	utime_t time = timer_get_time();
	
	while ((utime_t)(timer_get_time() - time) < tm)
		;
}

