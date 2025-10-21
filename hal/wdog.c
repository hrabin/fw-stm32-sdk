#include "common.h"
#include "os.h"
#include "os_task.h"
#include "hardware.h"
#include "wd.h"
#include "wdog.h"
#include "log.h"
#include "gpreg.h"

LOG_DEF("WDOG");

#define WDT_ENABLED 1

#define _UNKNOWN (-1)

typedef struct
{
    os_timer_t   limit;
    os_timer_t   period;
    u32         *stack;
    const ascii *name;
    int          stack_free;
    bool         suspend;

} wdog_task_guard_t;

static struct
{
    os_mutex_t mutex;
    wdog_task_id_t tasks_active;
    bool feed_enable;
    wdog_task_guard_t task_guard[WDOG_GUARDED_TASKS];

} _wdog;

static void _wdog_lock(void)
{
    OS_MUTEX_LOCK(_wdog.mutex);
}

static void _wdog_unlock(void)
{
    OS_MUTEX_UNLOCK(_wdog.mutex);
}

void wdog_init(void)
{
    wd_init();
    memset(&_wdog, 0, sizeof(_wdog));
    OS_MUTEX_INIT(_wdog.mutex);
}

void wdog_run(void)
{
    wd_run();
}

void wdog_reset (u32 reason)
{
    GPREG_WRITE(GPREG_WDID, GPREG_WDID_REBOOT_RQ);
    GPREG_WRITE(GPREG_BOOT, reason);
    wd_reset();
}

void wdog_disable (void)
{
    wd_disable();
}

void wdog_reset_info_set(u32 id)
{
    GPREG_WRITE(GPREG_INFO, id);
}

u32  wdog_reset_info(void)
{
    return (GPREG_INFO);
}

static void _die(void)
{
    while (1)
    {
        OS_DELAY(1);
    }
}

wdog_task_id_t wdog_task_register (const ascii *name, uint32_t max_period, uint32_t *stack)
{
    wdog_task_id_t id = WDOG_TASK_ID_NONE;

    wdog_task_guard_t *tg;

    if (max_period==0)
        return (id);

    _wdog_lock();

    if (_wdog.tasks_active < WDOG_GUARDED_TASKS)
    {
        id = _wdog.tasks_active++;

        tg = &_wdog.task_guard[id];

        tg->period = max_period;
        tg->limit  = 0; // temporary disabled, wait for first wdog_task_feed()
        tg->stack  = stack;
        tg->stack_free = _UNKNOWN;
        tg->name = name;
    }

    _wdog_unlock();

    if (id == WDOG_TASK_ID_NONE)
    {
        LOG_ERROR("Task not guarded, wdog full !");
    }
    else
    {
        LOG_DEBUG("registered task \"%s\", id=%d", name, id);
    }
    return (id);
}

void wdog_task_feed(wdog_task_id_t id)
{
    if (id >= _wdog.tasks_active)
    {
        return;
    }

    _wdog_lock();

    _wdog.task_guard[id].limit  = os_timer_get() + _wdog.task_guard[id].period;

    _wdog_unlock();
}

void wdog_task_suspend(wdog_task_id_t id)
{
    if (id >= _wdog.tasks_active)
    {
        return;
    }
    _wdog.task_guard[id].suspend = true;
}

void wdog_task_resume(wdog_task_id_t id)
{
    if (id >= _wdog.tasks_active)
    {
        return;
    }
    wdog_task_feed(id);
    _wdog.task_guard[id].suspend = false;
}

static int _wdog_stack_free(uint32_t *stack)
{
    int i;
    
    if (stack == NULL)
    {
        return (_UNKNOWN);
    }

    for (i=0; i<1024; i++)
    {
        if (stack[i] != 0xA5A5A5A5)
        {
            break;
        }
    }

    return (i*4);
}

void wdog_stack_info(buf_t *result)
{
    int i;

    _wdog_lock();

    for (i=0; i<_wdog.tasks_active; i++)
    {
        buf_append_fmt(result, "WDOG: task %d", i);

        if (_wdog.task_guard[i].name != NULL)
        {
            buf_append_fmt(result, " \"%s\"", _wdog.task_guard[i].name);
        }
        buf_append_fmt(result, ", free %d" NL, _wdog_stack_free(_wdog.task_guard[i].stack));
    }

    _wdog_unlock();
}

void wdog_main_task (void)
{   // to be called regularly, at least until WD_TIMEOUT
    // (in case _WDOG_OWN_TASK is disabled)
    static os_timer_t task_tm = 0;

    wdog_task_guard_t *tg;
    os_timer_t tm;
    wdog_task_id_t i;
    int tmp;

    tm = os_timer_get();

    if (tm < task_tm)
        return;

    task_tm = tm + 500*MS;

    _wdog_lock();

    for (i=0; i<_wdog.tasks_active; i++)
    {
        tg = &_wdog.task_guard[i];

        if ((tmp = _wdog_stack_free(tg->stack)) != tg->stack_free)
        {
            if (tmp == 0)
            {
                LOG_ERROR("task %d, stack overflow !", i);
                _wdog.feed_enable = false;
                _die();
            }
            if (tm > (10*SECOND))
                LOG_INFO("task %d, stack free %d ", i, tmp);

            tg->stack_free = tmp;
        }

        if (tg->limit == 0)
        {
            continue; // guarding disabled
        }

        if (tg->suspend)
        {
            continue; // guarding temporary suspended
        }

        if (tm < tg->limit)
        {
            continue; // OK
        }

        _wdog.feed_enable = false;

        LOG_ERROR("task %d, dead ! ", i);
        wdog_reset_info_set(GPREG_INFO_TASK_DEAD + i);

#if WDT_ENABLED != 0
       _die();
#else  // WDT_ENABLED != 0
        tg->limit  = tm + 10*1000*MS;
#endif // WDT_ENABLED == 0
    }
    _wdog_unlock();
    wd_feed();
}

