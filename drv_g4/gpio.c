#include "platform_setup.h"
#include "gpio.h"
#include "hardware.h"
#include "main.h"

void gpio_init (void)
{
	// Enable the peripheral clock for GPIO
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOCEN | RCC_AHB2ENR_GPIODEN | RCC_AHB2ENR_GPIOEEN | RCC_AHB2ENR_GPIOFEN;

}

