#include "os.h"

static os_timer_t _os_ticks = 0;


void vApplicationTickHook(void)
{
	static TickType_t xTickLast = 0;
	TickType_t xTickNow = xTaskGetTickCountFromISR();
	_os_ticks += xTickNow - xTickLast;
	xTickLast = xTickNow;
}

void os_critical_enter(void)
{
	__disable_irq();
	// taskENTER_CRITICAL();
}

void os_critical_exit(void)
{
	__enable_irq();
	// taskEXIT_CRITICAL();
}

os_timer_t os_timer_get(void)
{
	os_timer_t ret;

	os_critical_enter();
	ret = _os_ticks;
	os_critical_exit();

	return(ret);
}

__WEAK u32 HAL_GetTick(void)
{
	return (os_timer_get());
}
