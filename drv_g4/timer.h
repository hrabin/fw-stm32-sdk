#ifndef TIMER_H
#define TIMER_H

// #include    "hardware.h"

#define TIMER_MS    1000UL
#define TIMER_US    1UL

#define TIMER2_ON 1
#define TIMER3_ON 0
#define TIMER5_ON 0

typedef u32 timer_time_t;

#define timer_get_time()    timer2_get_time()
#define timer_start()       timer2_free_run()

typedef uint32_t utime_t;

void timer2_free_run(void);
void timer2_stop(void);
void timer2_run(void);
void timer2_irq_init(void);

#define timer2_get_time() (TIM2->CNT)

void timer3_free_run(void);

#define timer3_get_time() (TIM3->CNT)
void timer5_free_run(void);

#define timer5_get_time() (TIM5->CNT)

void timer_delay_us (utime_t tm);
void timer_delay_ms(u32 ms);

#endif // TIMER_H
