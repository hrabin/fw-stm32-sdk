#ifndef OS_TASK_H
#define OS_TASK_H

#include "platform_setup.h"
#include "cmsis_os.h"


#define _QUOTEME(x) #x
#define QUOTEME(x) _QUOTEME(x)

// FreeRTOS abstraction
#define OS_TASK_PRIO_LOW  osPriorityBelowNormal
#define OS_TASK_PRIO_NORM osPriorityNormal
#define OS_TASK_PRIO_HI   osPriorityAboveNormal

#define OS_TASK_DEF(NAME, STACK_SIZE, PRIORITY) \
    osThreadId_t NAME##_id;                     \
    const osThreadAttr_t NAME##_attr = {        \
        .name = _QUOTEME(NAME),                 \
        .stack_size = (STACK_SIZE),             \
        .priority = (osPriority_t) PRIORITY,    \
    };

#define OS_TASK_CREATE(NAME) NAME##_id=osThreadNew(NAME, NULL, & NAME##_attr)

#define OS_TASK_DEF_STATIC(NAME, STACK_SIZE, PRIORITY) \
    osThreadId_t NAME##_id;                     \
    u32 NAME##_stack[STACK_SIZE/4];             \
    StaticTask_t NAME##_cb_mem;                 \
    const osThreadAttr_t NAME##_attr = {        \
        .name = _QUOTEME(NAME),                 \
        .stack_size = (STACK_SIZE),             \
        .priority = (osPriority_t) PRIORITY,    \
        .cb_mem = &NAME##_cb_mem,               \
        .cb_size = sizeof(StaticTask_t),        \
        .stack_mem = NAME##_stack,              \
        .stack_size = STACK_SIZE                \
    };

#define OS_TASK_CREATE_STATIC(NAME) { \
    memset(NAME##_stack, 0xA5, sizeof(NAME##_stack)); \
    NAME##_id=osThreadNew(NAME, NULL, & NAME##_attr); \
}

#define OS_WAIT_TO_DEFINE   static os_timer_t next_time; \
                        static s32 wait_time

#define OS_WAIT_TO_SET(period) next_time=os_timer_get()

#define OS_WAIT_TO(period)  { \
                        next_time+=(period)*MS;               \
                        wait_time=next_time-os_timer_get();   \
                        if ((wait_time>=MS)                   \
                         && (wait_time<=(period+2)*MS))       \
                        {                                     \
                            OS_DELAY(wait_time/MS);           \
                        }                                     \
                        else                                  \
                        {                                     \
                            next_time=os_timer_get();         \
                        }                                     \
                    }

#define OS_TASK(name)  static void name (void *args)
#define OS_INIT() osKernelInitialize()
#define OS_TASK_EXIT()
#define OS_START() osKernelStart()
#define OS_TASK_YIELD() osThreadYield()

typedef struct
{
    osSemaphoreId_t id;
    osSemaphoreAttr_t attr;

} os_semaphore_t;

typedef struct
{
    osMutexId_t id;
    osMutexAttr_t attr;

} os_mutex_t;

#define OS_SEMAPHORE(s)      os_semaphore_t s
#define OS_SEMAPHORE_INIT(s) {s.attr.name=QUOTEME(s); s.id=osSemaphoreNew(1,1, &s.attr); }
#define OS_SEMAPHORE_TAKE(s) osSemaphoreAcquire((s.id), osWaitForever)
#define OS_SEMAPHORE_TAKE_SOFT(s,tmout) (osSemaphoreAcquire(s.id,tmout)!=0)
#define OS_SEMAPHORE_GIVE(s) osSemaphoreRelease(s.id)

#define OS_MUTEX(m)         os_mutex_t m
#define OS_MUTEX_INIT(m)    {m.attr.name=QUOTEME(m); m.id=osMutexNew(&((m).attr));}
#define OS_MUTEX_LOCK(m)    osMutexAcquire((m).id,osWaitForever)
#define OS_MUTEX_UNLOCK(m)  osMutexRelease((m).id)

#endif // ! OS_TASK_H

