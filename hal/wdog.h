#ifndef WDOG_H
#define WDOG_H

#include "type.h"
#include "buf.h"

#define	WDOG_GUARDED_TASKS (7)

typedef u8 wdog_task_id_t;
#define	WDOG_TASK_ID_NONE (0xFF)

void wdog_init (void);
void wdog_run (void);
void wdog_reset (u32 reason);
void wdog_disable (void);
void wdog_reset_info_set(u32 id);
u32  wdog_reset_info(void);

wdog_task_id_t wdog_task_register (const ascii *name, uint32_t max_period, uint32_t *stack);
void wdog_task_feed(wdog_task_id_t id);
void wdog_task_suspend(wdog_task_id_t id);
void wdog_task_resume(wdog_task_id_t id);
void wdog_stack_info(buf_t *result);
void wdog_main_task (void);


#endif // ! WDOG_H
