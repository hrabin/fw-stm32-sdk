#ifndef ADC_H
#define ADC_H

#include "type.h"

#define ADC_MEAS_PERIOD (50) // [ms]

#define ADC_VALUE_NONE UINT32_MAX

typedef enum 
{
    ADC_CH_MAIN,// PWR_MAIN_MEAS
    ADC_CH_BATT,// PWR_BAT_MEAS
    ADC_CH_V4,  // PWR_V4_MEAS
    ADC_CH_VBAT,// int sensor
    ADC_CH_REF, // int sensor
    ADC_CH_TEMP,// int sensor

    ADC_CHANNELS
} adc_channel_e;

extern bool adc_initialized;

bool adc_init(void);

s32 adc_get_value(adc_channel_e channel);

#define AD_VALUE(chnl)     adc_get_value(chnl)

bool adc_task(void);

#endif // ! ADC_H
