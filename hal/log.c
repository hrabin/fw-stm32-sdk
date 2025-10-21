#include "os.h"
#include "log.h"

#if LOG_ENABLE

#include <stdarg.h>

#if LOG_LOCK == 1

OS_MUTEX(log_mutex);

int log_debug_level = LOG_DEBUG_LEVEL_DEFAULT;
int log_debug_select = 0;

void log_init(void)
{
    OS_MUTEX_INIT(log_mutex);
}

void log_lock(void)
{
    OS_MUTEX_LOCK(log_mutex);
}

void log_unlock(void)
{
    OS_MUTEX_UNLOCK(log_mutex);
}
#endif // LOG_LOCK == 1

static void _log_timestamp(void)
{
    u64 t = OS_TIMER()/OS_TIMER_MS;
    OS_PRINTF("# %ld ",(u32)t);
}

void log_msg(const ascii *type, const ascii *id, const ascii *fmt, ... )
{
    va_list args;

    va_start(args, fmt);

    log_lock();

    _log_timestamp();
    OS_PRINTF("%s(%s): ", type, id);
    vprintf(fmt, args);
    va_end(args);
    OS_PRINTF(NL);
    OS_FLUSH();

    log_unlock();
}

void log_msg_debug(int debug_level, const ascii *id, const ascii *fmt, ... )
{
    va_list args;

    if ((debug_level > log_debug_level) 
     && (debug_level != log_debug_select))
        return;

    va_start(args, fmt);

    log_lock();

    _log_timestamp();
    OS_PRINTF("D%d(%s): ", debug_level, id);
    vprintf(fmt, args);
    va_end(args);
    OS_PRINTF(NL);
    OS_FLUSH();

    log_unlock();
}

void log_dump(const ascii *id, const ascii *text, const u8 *data, int data_len)
{
    int n = 0;
    
    log_lock();

    _log_timestamp();
    OS_PRINTF("X(%s) %s:", id, text);
    
    if (data_len > 512)
    {
        OS_PRINTF("(%d)", data_len);
        data_len = 512;
    }

    if (data_len > 32)
         OS_PUTTEXT(NL);

    while (data_len--)
    {
        OS_PRINTF(" %x%x", *data >> 4, *data & 0xf);
        data++;
        
        n++;
        if (n == 16)
        {
            OS_PUTTEXT(" ");
        }
        else if (n == 32)
        {
            OS_PUTTEXT(NL);
            n = 0;
        }
    }
    OS_PRINTF(NL);
    OS_FLUSH();
    log_unlock();
}

#endif // LOG_ENABLE

