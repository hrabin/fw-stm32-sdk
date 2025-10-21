#include "common.h"

#include "shock.h"
#include "system.h"
#include "alarm.h"

#if SHOCK_ENABLE == 1

#include "lis2dh12.h"
#include "log.h"
LOG_DEF("SHOCK");

#define _TASK_PERIOD 10 // [ms]
#define _ACTIVATION_TIME  (3 * (1000/_TASK_PERIOD)) // [periods]

typedef struct {

    s32  x_avg;
    s32  y_avg;
    s32  z_avg;
    s32  x_avg_f;
    s32  y_avg_f;
    s32  z_avg_f;
    u32  diff_avg;
    s16  x;
    s16  y;
    s16  z;
    s16  x_old;
    s16  y_old;
    s16  z_old;
    s16  alarm_cnt;
    u16  alarm_cnt_trsh;
    u16  alarm_treshold;
    u16  alarm_timer;
    bool reaction_alarm_enabled;
    bool reaction_warning_enabled;

} shock_t;

static shock_t _shock;

bool shock_debug = false;

#if defined HW_ACCL_SPI_MUTEX
    HW_ACCL_SPI_MUTEX_DEF;
#endif // defined HW_ACCL_SPI_MUTEX

static void _shock_spi_lock(void)
{
#if defined HW_ACCL_SPI_MUTEX
    OS_MUTEX_LOCK(HW_ACCL_SPI_MUTEX);
#endif // defined HW_ACCL_SPI_MUTEX
}

static void _shock_spi_unlock(void)
{
#if defined HW_ACCL_SPI_MUTEX
    OS_MUTEX_UNLOCK(HW_ACCL_SPI_MUTEX);
#endif // defined HW_ACCL_SPI_MUTEX
}

bool shock_init (void)
{
    memset(&_shock, 0, sizeof(_shock));
    lis2dh12_init();
    return (shock_reinit());
}

bool shock_detected (void)
{
    return (_shock.alarm_timer > 0 ? true : false);
}

u32 shock_get_activity (void)
{
    return (_shock.diff_avg);
}

static void shock_evaluate (void)
{   // 
    u32 tmp;
    s32 dx,dy,dz;

    dx = _shock.x - _shock.x_old;
    _shock.x_old = _shock.x;

    dy = _shock.y - _shock.y_old;
    _shock.y_old = _shock.y;

    dz = _shock.z - _shock.z_old;
    _shock.z_old = _shock.z;

    dx = ABS(dx);
    dy = ABS(dy);
    dz = ABS(dz);

   
    if (shock_debug)
    {
        static os_timer_t tm = 0;
        static u32 dxm=0,dym=0,dzm=0;
        os_timer_t now = os_timer_get();

        if (dx > dxm) dxm = dx;
        if (dy > dym) dym = dy;
        if (dz > dzm) dzm = dz;
        
        if (now > tm)
        {
            tm = now + 1*OS_TIMER_SECOND;

            OS_PRINTF(NL);
            OS_PRINTF("SH: x=%d,y=%d,z=%d; dx=%ld,dy=%ld,dz=%ld, a=%d", _shock.x, _shock.y, _shock.z, dxm, dym, dzm, _shock.alarm_cnt);
            dxm=0;dym=0;dzm=0;
        }
    }

    if (_shock.alarm_cnt>0)
        _shock.alarm_cnt--;

    if (dx > _shock.alarm_treshold)
        _shock.alarm_cnt += 2;

    if (dy > _shock.alarm_treshold)
        _shock.alarm_cnt += 2;

    if (dz > _shock.alarm_treshold)
        _shock.alarm_cnt += 2;

    /*if (_shock.alarm_timer)
    {
        _shock.alarm_cnt = 0;
    }
    else*/
    if (_shock.alarm_cnt > _shock.alarm_cnt_trsh)
    {
        _shock.alarm_cnt = 0;
        if (_shock.alarm_timer == 0)
        {
            LOG_DEBUG("activation %ld", dx+dy+dz);
        }
        _shock.alarm_timer = _ACTIVATION_TIME;
    }

    // move direction change detect
    // we can dynamic update GNSS sending period
    // (1G is aprox 16000)
#define _AVG_SHIFT   (8) // slow average 
#define _AVG_F_SHIFT (4) // fast average

    // fast averaging from number of (1<<_AVG_F_SHIFT) samples
    _shock.x_avg_f -= (_shock.x_avg_f >> _AVG_F_SHIFT);
    _shock.x_avg_f += _shock.x_old;
    _shock.y_avg_f -= (_shock.y_avg_f >> _AVG_F_SHIFT);
    _shock.y_avg_f += _shock.y_old;
    _shock.z_avg_f -= (_shock.z_avg_f >> _AVG_F_SHIFT);
    _shock.z_avg_f += _shock.z_old;
    
    dx = (_shock.x_avg >> _AVG_SHIFT) - (_shock.x_avg_f >> _AVG_F_SHIFT);
    dx = ABS(dx);

    dy = (_shock.y_avg >> _AVG_SHIFT) - (_shock.y_avg_f >> _AVG_F_SHIFT);
    dy = ABS(dy);

    dz = (_shock.z_avg >> _AVG_SHIFT) - (_shock.z_avg_f >> _AVG_F_SHIFT);
    dz = ABS(dz);

    // slow averaging from number of (1<<_AVG_SHIFT) samples
    _shock.x_avg -= (_shock.x_avg >> _AVG_SHIFT);
    _shock.x_avg += _shock.x_old;
    _shock.y_avg -= (_shock.y_avg >> _AVG_SHIFT);
    _shock.y_avg += _shock.y_old;
    _shock.z_avg -= (_shock.z_avg >> _AVG_SHIFT);
    _shock.z_avg += _shock.z_old;
    
    tmp = _shock.diff_avg - (_shock.diff_avg >> 3);
    tmp += (dx + dy + dz);
    
    _shock.diff_avg = tmp;
}

void shock_task (void)
{   // here we rely on calling this task fast enough (every 10ms)
    bool data_valid = false;

    if (_shock.alarm_timer > 0) 
        _shock.alarm_timer--;

    _shock_spi_lock();

    if (lis2dh12_read_data(&_shock.x, &_shock.y, &_shock.z))
    {
        data_valid = true;
    }

    _shock_spi_unlock();

    if (data_valid)
        shock_evaluate();

    return;
}

bool shock_reinit (void)
{
    _shock.x = 0;
    _shock.y = 0;
    _shock.z = 0;
    _shock.x_old = 0;
    _shock.y_old = 0;
    _shock.z_old = 0;
    _shock.x_avg = 0;
    _shock.y_avg = 0;
    _shock.z_avg = 0;
    _shock.x_avg_f = 0;
    _shock.y_avg_f = 0;
    _shock.z_avg_f = 0;
    _shock.diff_avg = 0;
    _shock.alarm_cnt = 0;
    _shock.alarm_timer = 0;

    shock_set_param(8);

    return (lis2dh12_start());
}

void shock_set_param (u8 sensitivity)
{   //
    const u16 SEN_ALARM_TRSH_TABLE[] = {950,800,650,550,500,450,400,350,320};   // sensitivity treshold
    const u16 SEN_ALARM_CNTT_TABLE[] = { 48, 44, 40, 36, 32, 28, 24, 20, 16};   // number of impulses to activate

    _shock.reaction_alarm_enabled = false;   
    _shock.reaction_warning_enabled = false; 

    if (sensitivity >= sizeof(SEN_ALARM_TRSH_TABLE)/sizeof(u16))
        return;

    _shock.alarm_treshold = SEN_ALARM_TRSH_TABLE[sensitivity];
    _shock.alarm_cnt_trsh = SEN_ALARM_CNTT_TABLE[sensitivity];
}

#else // SHOCK_ENABLE
  #warning "NO ACCELEROMETER"
#endif // ! SHOCK_ENABLE


