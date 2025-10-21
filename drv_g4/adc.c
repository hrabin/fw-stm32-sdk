#include "os.h"
#include "hardware.h"
#include "adc.h"
#include "irq.h"

#include "stm32g4xx_ll_adc.h"
#include "stm32g4xx_ll_bus.h"
#include "stm32g4xx_ll_rcc.h"

bool adc_initialized = false;

#define ADC_TASK_PERIOD (ADC_MEAS_PERIOD * OS_TIMER_MS)

static int adc_channel_ptr = 0;
static u16 adc_data[ADC_CHANNELS];
static s32 adc_values[ADC_CHANNELS];
static bool adc_running = false;

typedef struct {
    u8  channel;
    s16 add;
    u32 multiplier;
} adc_channel_setup_t;

const adc_channel_setup_t adc_channel_setup[ADC_CHANNELS] = {
    [ADC_CH_MAIN] = {HW_ADC_MAIN_MEAS_CHNL,    0, HW_ADC_MAIN_MEAS_MULT},
    [ADC_CH_BATT] = {HW_ADC_BAT_MEAS_CHNL,     0, HW_ADC_BAT_MEAS_MULT},
    [ADC_CH_V4]   = {HW_ADC_V4_MEAS_CHNL,      0, HW_ADC_V4_MEAS_MULT}, // ADC_CH_V4
    [ADC_CH_VBAT] = {17, 0, 3000}, // ADC1.17, div 3
    [ADC_CH_REF]  = {18, 0, 1000}, // ADC1.18, 3.0V +-10mV
    [ADC_CH_TEMP] = {16, 0, 10*1000}  // ADC1.16, TS_CAL1 TS_CAL2
};
// VREFINT_CAL = 1648, TS_CAL1 = 1029, TS_CAL2 = 1365
//
// Temp = 80/(TS_CAL2-TS_CAL1)*(value-TS_CAL1)+30
//
// Vref+ = VREFINT_CAL_ADDR * VREFINT_CAL_VREF / ADC_CH_REF

/*
VREFINT_CAL_ADDR          ADC raw data acquired at temperature 30 DegC (tolerance: +-5 DegC), Vref+ = 3.0 V (tolerance: +-10 mV).
VREFINT_CAL_VREF          Analog voltage reference (Vref+) value with which temperature sensor has been calibrated in production (tolerance: +-10 mV) (unit: mV).
TEMPSENSOR_CAL1_ADDR      address of parameter TS_CAL1: On STM32G4, temperature sensor ADC raw data acquired at temperature  30 DegC (tolerance: +-5 DegC), Vref+ = 3.0 V (tolerance: +-10 mV).
TEMPSENSOR_CAL2_ADDR      address of parameter TS_CAL2: On STM32G4, temperature sensor ADC raw data acquired at temperature 110 DegC (tolerance: +-5 DegC), Vref+ = 3.0 V (tolerance: +-10 mV).
TEMPSENSOR_CAL1_TEMP      temperature at which temperature sensor has been calibrated in production for data into TEMPSENSOR_CAL1_ADDR (tolerance: +-5 DegC) (unit: DegC).
TEMPSENSOR_CAL2_TEMP      temperature at which temperature sensor has been calibrated in production for data into TEMPSENSOR_CAL2_ADDR (tolerance: +-5 DegC) (unit: DegC).
TEMPSENSOR_CAL_VREFANALOG Analog voltage reference (Vref+) voltage with which temperature sensor has been calibrated in production (+-10 mV) (unit: mV).
*/

static void _adc_channel_setup(u8 channel)
{
    LL_ADC_SetChannelSamplingTime(ADC1, channel, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSingleDiff(ADC1, channel, LL_ADC_SINGLE_ENDED);
}

static void _adc_channel_select(u8 channel)
{
    ADC1->SQR1 = adc_channel_setup[channel].channel << ADC_SQR1_SQ1_Pos;
}

static void _adc_start(void)
{
    adc_running = true;
    adc_channel_ptr = 0;
    _adc_channel_select(adc_channel_ptr);
    LL_ADC_REG_StartConversion(ADC1);
}

bool adc_init(void)
{
    u32 i;

    LL_ADC_InitTypeDef ADC_InitStruct = {0};
    LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
    LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};

    memset (&adc_data, 0, sizeof(adc_data));

    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    GPIO_MODE(GPIOA, 0, GPIO_MODE_ANALOG);
    GPIO_PUPDN(GPIOA, 0, GPIO_PUPDN_NONE);

    GPIO_MODE(GPIOA, 1, GPIO_MODE_ANALOG);
    GPIO_PUPDN(GPIOA, 1, GPIO_PUPDN_NONE);
    
    GPIO_MODE(GPIOA, 2, GPIO_MODE_ANALOG);
    GPIO_PUPDN(GPIOA, 2, GPIO_PUPDN_NONE);

    LL_RCC_SetADCClockSource(LL_RCC_ADC12_CLKSOURCE_SYSCLK);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_ADC12); // RCC->AHB2ENR |= RCC_AHB2ENR_ADC12EN;

    // Common config
    ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
    ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
    ADC_InitStruct.LowPowerMode = LL_ADC_LP_MODE_NONE;
    LL_ADC_Init(ADC1, &ADC_InitStruct);

    ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
    ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE;
    ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
    ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
    ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_NONE;
    ADC_REG_InitStruct.Overrun = LL_ADC_REG_OVR_DATA_PRESERVED;
    LL_ADC_REG_Init(ADC1, &ADC_REG_InitStruct);
    
    ADC_CommonInitStruct.CommonClock = LL_ADC_CLOCK_ASYNC_DIV16;
    ADC_CommonInitStruct.Multimode = LL_ADC_MULTI_INDEPENDENT;
    LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(ADC1), &ADC_CommonInitStruct);

    LL_ADC_SetGainCompensation(ADC1, 0);
    LL_ADC_SetOverSamplingScope(ADC1, LL_ADC_OVS_DISABLE);

    // set sampling time
    ADC1->SMPR1 = ADC_SMPR1_SMP1_2 |  ADC_SMPR1_SMP2_1 |  ADC_SMPR1_SMP3_1 |  ADC_SMPR1_SMP4_1
                | ADC_SMPR1_SMP5_1 |  ADC_SMPR1_SMP6_1 |  ADC_SMPR1_SMP7_1 |  ADC_SMPR1_SMP8_1 |  ADC_SMPR1_SMP9_1;

    ADC1->SMPR2 = ADC_SMPR2_SMP10_1 | ADC_SMPR2_SMP11_1 | ADC_SMPR2_SMP12_1 | ADC_SMPR2_SMP13_1
                | ADC_SMPR2_SMP14_1 | ADC_SMPR2_SMP15_1 | ADC_SMPR2_SMP16_1 | ADC_SMPR2_SMP17_1 | ADC_SMPR2_SMP18_1;
    
    // LL_ADC_SetSamplingTimeCommonConfig(ADC1, );

    // Disable ADC deep power down (enabled by default after reset state)
    LL_ADC_DisableDeepPowerDown(ADC1);
    // Enable ADC internal voltage regulator
    LL_ADC_EnableInternalRegulator(ADC1);
    // Delay for ADC internal voltage regulator stabilization.
    // Compute number of CPU cycles to wait for, from delay in us.
    // Note: Variable divided by 2 to compensate partially
    // CPU processing cycles (depends on compilation optimization).
    // Note: If system core clock frequency is below 200kHz, wait time
    // is only a few CPU processing cycles.
    i = ((LL_ADC_DELAY_INTERNAL_REGUL_STAB_US * (SystemCoreClock / (100000 * 2))) / 10);
    
    while(i != 0)
    {
        i--;
    }

    LL_ADC_StartCalibration(ADC1, LL_ADC_SINGLE_ENDED);
    OS_DELAY(1);
    
    // connect internal sensors
    LL_ADC_SetCommonPathInternalCh(ADC12_COMMON, LL_ADC_PATH_INTERNAL_VREFINT | LL_ADC_PATH_INTERNAL_TEMPSENSOR | LL_ADC_PATH_INTERNAL_VBAT);

    ADC1->SQR1 = 0;

    for (i=0; i<ADC_CHANNELS; i++)
    {
        _adc_channel_setup(adc_channel_setup[i].channel);
        adc_values[i] = ADC_VALUE_NONE;
    }

    // enable IRQ (EOC == End Of Conversion)
    LL_ADC_EnableIT_EOC(ADC1);
    irq_enable(ADC1_2_IRQn, IRQ_PRIO_ADC);

    LL_ADC_Enable(ADC1);
    // LL_ADC_REG_StartConversion(ADC1);

    adc_initialized = true;

    return (true);
}

void ADC1_2_IRQHandler(void)
{
    u32 status = ADC1->ISR;

    if ((status & ADC_ISR_EOC) == 0)
        return;

    ADC1->ISR |= ADC_ISR_EOC; // clear irq flag (writting '1' means clear)
    
    // end of single conversion

    if (adc_channel_ptr < ADC_CHANNELS)
    {
        adc_data[adc_channel_ptr] = ADC1->DR;
        adc_channel_ptr++;

        if (adc_channel_ptr < ADC_CHANNELS)
        {   // switch to next channel and start conversion
            _adc_channel_select(adc_channel_ptr);
            LL_ADC_REG_StartConversion(ADC1);
        }
        else
        {   // all channels done, wait for manual start
            adc_running = false;
        }
    }
}

s32 adc_get_value(adc_channel_e channel)
{
    if (channel >= ADC_CHANNELS)
        return (ADC_VALUE_NONE);

    return (adc_values[channel]);
}

static __inline u32 _get_vref(u32 value)
{  
    return (*VREFINT_CAL_ADDR * VREFINT_CAL_VREF / value);
}

bool adc_task(void)
{
    static os_timer_t task_timer = 0;

    os_timer_t now = os_timer_get();

    if (adc_running)
        return (false);

    if (now < task_timer)
        return (false);

    task_timer = now + ADC_TASK_PERIOD;

    if (! adc_initialized)
        return (false);

    if (adc_channel_ptr == ADC_CHANNELS)
    {
        u32 vref = _get_vref(adc_data[ADC_CH_REF]);
        int i;

        for (i=0; i<ADC_CHANNELS; i++)
        {
            s32 tmp = adc_data[i];

            // compute value according to HW resistor divisor
            tmp = tmp * adc_channel_setup[i].multiplier / 1000 + adc_channel_setup[i].add;
            // update according to current voltage reference (vref)
            tmp = vref * tmp / 4096; // [mV]

            // TODO: value calibration
            adc_values[i] = tmp;
        }
        _adc_start();
        return (true);
    }

    _adc_start();
    return (false);
}

void adc_test(void)
{
    int i;
    float value;
    s16 t;

    OS_PRINTF(NL);
    for (i=0; i<ADC_CHANNELS; i++)
    {
        OS_PRINTF("A%d: %lu" NL, i, adc_values[i]);
    }

    value = adc_values[ADC_CH_TEMP]*4096/TEMPSENSOR_CAL_VREFANALOG;
    value /= 10; // value stored x10 due to precision 
    value = (80.f*(value-*TEMPSENSOR_CAL1_ADDR)/(*TEMPSENSOR_CAL2_ADDR-*TEMPSENSOR_CAL1_ADDR))+30;
    t = value*10;

    OS_PRINTF("TEMP: %d.%d C" NL, t/10, t%10);
}


