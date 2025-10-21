#include "platform_setup.h"
#include "hardware.h"

#include "spi.h"
#include "gpio.h"

#include <stm32g4xx_ll_spi.h>
#include <stm32g4xx_ll_bus.h>

#define GPIO_AF6_SPI3          ((uint8_t)0x06)  /* SPI3 Alternate Function mapping       */

static void _spi_pin_init(gpio_port_t *port, u32 pin)
{
	GPIO_AFR(port, pin, GPIO_AF_SPI);
	GPIO_MODE(port, pin, GPIO_MODE_ALT);
	GPIO_SPEED(port, pin, GPIO_SPEED_LOW);
	GPIO_PUPDN(port, pin, GPIO_PUPDN_NONE);
}

#if SPI1_ON

void spi1_init(void)
{
	LL_SPI_InitTypeDef SPI_InitStruct = {0};

	// Peripheral clock enable
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SPI1EN);

	// SPI1 GPIO Configuration
	// PA5   ------> SPI1_SCK
	// PA6   ------> SPI1_MISO
	// PA7   ------> SPI1_MOSI

	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN);
	_spi_pin_init(GPIOA, 5);
	_spi_pin_init(GPIOA, 6);
	_spi_pin_init(GPIOA, 7);

	// SPI1 parameter configuration
	SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
	SPI_InitStruct.Mode              = LL_SPI_MODE_MASTER;
	SPI_InitStruct.DataWidth         = LL_SPI_DATAWIDTH_8BIT;
	SPI_InitStruct.ClockPolarity     = LL_SPI_POLARITY_LOW;
	SPI_InitStruct.ClockPhase        = LL_SPI_PHASE_1EDGE;
	SPI_InitStruct.NSS               = LL_SPI_NSS_SOFT;
	SPI_InitStruct.BaudRate          = LL_SPI_BAUDRATEPRESCALER_DIV4;
	SPI_InitStruct.BitOrder          = LL_SPI_MSB_FIRST;
	SPI_InitStruct.CRCCalculation    = LL_SPI_CRCCALCULATION_DISABLE;
	SPI_InitStruct.CRCPoly           = 7;
	LL_SPI_Init(SPI1, &SPI_InitStruct);

	// enable SPI1
	SPI1->CR1 |= SPI_CR1_SPE;

	while ((SPI1->SR & SPI_SR_TXE) == 0)
		;
}

unsigned char spi1_transfer(unsigned char c)
{
 	while((SPI1->SR & SPI_SR_TXE) == 0)
 		;

	*((__IO u8 *)&SPI1->DR) = c;
	
 	while((SPI1->SR & SPI_SR_RXNE) == 0)
 		;

	return SPI1->DR;
}

#endif // SPI1_ON

#if SPI2_ON

void spi2_init(void)
{
	LL_SPI_InitTypeDef SPI_InitStruct = {0};

	// Peripheral clock enable
	SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_SPI2EN);

	// SPI2 GPIO Configuration
	// PB13  ------> SPI2_SCK
	// PB14  ------> SPI2_MISO
	// PB15  ------> SPI2_MOSI

	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN);

	_spi_pin_init(GPIOF, 1);
	_spi_pin_init(GPIOB, 14);
	_spi_pin_init(GPIOB, 15);

	// SPI parameter configuration
	SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
	SPI_InitStruct.Mode              = LL_SPI_MODE_MASTER;
	SPI_InitStruct.DataWidth         = LL_SPI_DATAWIDTH_8BIT;
	SPI_InitStruct.ClockPolarity     = LL_SPI_POLARITY_LOW;
	SPI_InitStruct.ClockPhase        = LL_SPI_PHASE_1EDGE;
	SPI_InitStruct.NSS               = LL_SPI_NSS_SOFT;
	SPI_InitStruct.BaudRate          = LL_SPI_BAUDRATEPRESCALER_DIV4;
	SPI_InitStruct.BitOrder          = LL_SPI_MSB_FIRST;
	SPI_InitStruct.CRCCalculation    = LL_SPI_CRCCALCULATION_DISABLE;
	SPI_InitStruct.CRCPoly           = 7;
	LL_SPI_Init(SPI2, &SPI_InitStruct);

	// enable SPI
	SPI2->CR1 |= SPI_CR1_SPE;

	while ((SPI2->SR & SPI_SR_TXE) == 0)
		;
}

unsigned char spi2_transfer(unsigned char c)
{
 	while((SPI2->SR & SPI_SR_TXE) == 0)
 		;

	*((__IO u8 *)&SPI2->DR) = c;
	
 	while((SPI2->SR & SPI_SR_RXNE) == 0)
 		;

	return SPI2->DR;
}

#endif // SPI2_ON

#if SPI3_ON

void spi3_init(void)
{
	// SPI3 parameter configuration
	GPIO_AFR(GPIOC, 10, GPIO_AF6_SPI3);
	GPIO_AFR(GPIOC, 11, GPIO_AF6_SPI3);
	GPIO_AFR(GPIOC, 12, GPIO_AF6_SPI3);

	GPIO_MODE(GPIOC, 10, GPIO_MODE_ALT);
	GPIO_MODE(GPIOC, 11, GPIO_MODE_ALT);
	GPIO_MODE(GPIOC, 12, GPIO_MODE_ALT);

	// Speed mode configuration
	GPIO_SPEED(GPIOC, 10, GPIO_SPEED_HIGH);
	GPIO_SPEED(GPIOC, 11, GPIO_SPEED_HIGH);
	GPIO_SPEED(GPIOC, 12, GPIO_SPEED_HIGH);

  	// Enable the SPI peripheral
	RCC->APB1ENR1 |= RCC_APB1ENR1_SPI3EN;

	SPI3->CR2 = 0; // SPI_CR2_SSOE;

#define	SPI3_BAUD_DIVIDER_REG (1<<3)
	SPI3->CR1 = SPI_CR1_MSTR | SPI_CR1_SPE |SPI_CR1_SSI | SPI3_BAUD_DIVIDER_REG | SPI_CR1_SSM | SPI_CR1_CPOL | SPI_CR1_CPHA;
}

unsigned char spi3_transfer(unsigned char c)
{
	*((volatile unsigned char*)&SPI3->DR)=c;
	
	while((SPI3->SR & SPI_SR_RXNE)==0)
		;

	return SPI3->DR;
// 	return (0);
}

#endif // SPI3_ON


#if SPI5_ON

void spi5_init(void)
{

	GPIO_AFR(GPIOA, 10, GPIO_AF6_SPI5);
	GPIO_AFR(GPIOA, 12, GPIO_AF6_SPI5);
	GPIO_AFR(GPIOB,  0, GPIO_AF6_SPI5);

	GPIO_MODE(GPIOA, 10, GPIO_MODE_ALT);
	GPIO_MODE(GPIOA, 12, GPIO_MODE_ALT);
	GPIO_MODE(GPIOB,  0, GPIO_MODE_ALT);

	// Speed mode configuration
	GPIO_SPEED(GPIOA, 10, GPIO_SPEED_HIGH);
	GPIO_SPEED(GPIOA, 12, GPIO_SPEED_HIGH);
	GPIO_SPEED(GPIOB,  0, GPIO_SPEED_HIGH);

	// Enable the SPI peripheral
	RCC->APB2ENR |= RCC_APB2ENR_SPI5EN;
	SPI5->CR2 = 0; // SPI_CR2_SSOE;
	// SPI5->CR1=SPI_CR1_MSTR | SPI_CR1_SPE |SPI_CR1_SSI | SPI_CR1_SSM ; //bitrate PCLK/2 .. 2 nebo 4 MHz
#define	SPI5_BAUD_DIVIDER_REG (0<<3)
	SPI5->CR1 = SPI_CR1_MSTR | SPI_CR1_SPE |SPI_CR1_SSI | SPI5_BAUD_DIVIDER_REG | SPI_CR1_SSM /*| SPI_CR1_CPOL | SPI_CR1_CPHA*/; //bitrate PCLK/64 

}

unsigned char spi5_transfer(unsigned char c)
{
	*((volatile unsigned char*)&SPI5->DR)=c;
	
	while((SPI5->SR & SPI_SR_RXNE)==0)
		;

	return SPI5->DR;
}

#endif // SPI5_ON

