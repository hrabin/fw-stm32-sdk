#ifndef GPIO_H
#define	GPIO_H

// #include "stm32g4xx_ll_gpio.h"
// #include "stm32g4xx_hal_gpio.h"
// #include "stm32g4xx_hal_gpio_ex.h"

#define	gpio_port_t GPIO_TypeDef

//	GPIOx->MODER    - basic register mode (GPIO_MODE_*)
#define	GPIO_MODE_BITS             (2UL)
#define	GPIO_MODE_MASK             ((1UL<<GPIO_MODE_BITS)-1)
#define	GPIO_MODE_INPUT            (0)
#define	GPIO_MODE_OUTPUT           (1U)
#define	GPIO_MODE_ALT              (2U)
#define	GPIO_MODE_ANALOG           (3UL)
// GPIO_AF7_USART1

// "p" je port ve formatu "GPIOA"
#define	GPIO_MODE(p,bit,mode)  { p->MODER = (p->MODER                                \
                                  & ~(GPIO_MODE_MASK << ((bit) * GPIO_MODE_BITS)))   \
                                  | (mode << ((bit) * GPIO_MODE_BITS)); }

//	GPIOx->PUPDR    - pull-up/pull-down setting register (GPIO_PULLUPDN_*)
#define	GPIO_PUPDN_BITS            (2UL)
#define	GPIO_PUPDN_MASK            ((1UL<<GPIO_PUPDN_BITS)-1)
#define	GPIO_PUPDN_NONE            (0)
#define	GPIO_PUPDN_UP              (1)
#define	GPIO_PUPDN_DN              (2)


#define	GPIO_PUPDN(p,bit,mode) { p->PUPDR = (p->PUPDR                                \
                                  & ~(GPIO_PUPDN_MASK << ((bit) * GPIO_PUPDN_BITS))) \
                                  | (mode << ((bit) * GPIO_PUPDN_BITS)); }

// GPIOx->AFR[] - alternate function register
#define	GPIO_AFR(p,bit,mode) { p->AFR[(bit)>>3] = (p->AFR[(bit)>>3] \
                                  & ~(0xFUL << (((bit)&0x7)<<2)))   \
                                  | ((mode) << (((bit)&0x7)<<2)); }


//	GPIOx->OSPEEDR  - I/O output speed
#define GPIO_SPEED_BITS           (2UL) 
#define GPIO_SPEED_MASK           (3UL) 
#define GPIO_SPEED_LOW            0 // GPIO_SPEED_FREQ_LOW // IO works at 2 MHz
#define GPIO_SPEED_MEDIUM         1 // GPIO_SPEED_FREQ_MEDIUM  // range 12,5 MHz to 50 MHz
#define GPIO_SPEED_FAST           2 // GPIO_SPEED_FREQ_HIGH // range 25 MHz to 100 MHz
#define GPIO_SPEED_HIGH           3 // GPIO_SPEED_FREQ_VERY_HIGH  // range 50 MHz to 200 MHz

#define	GPIO_SPEED(p,bit,mode)  { p->OSPEEDR = (p->OSPEEDR                           \
                                  & ~(GPIO_SPEED_MASK << ((bit) * GPIO_SPEED_BITS)))   \
                                  | (mode << ((bit) * GPIO_SPEED_BITS)); }


//	GPIOx->OTYPER   - output typ push-pull/open-drain
#define	GPIO_OUTPUT_TYPE_BITS      (1UL)
#define	GPIO_OUTPUT_TYPE_MASK      ((1UL<<GPIO_OUTPUT_TYPE_BITS)-1)
#define	GPIO_OUTPUT_TYPE_PUSHPULL  (0)
#define	GPIO_OUTPUT_TYPE_OPENDRAIN (1)

#define	GPIO_OUTPUT_TYPE(p,bit,mode)  { p->OTYPER = (p->OTYPER                           \
                                  & ~(GPIO_OUTPUT_TYPE_MASK << ((bit) * GPIO_OUTPUT_TYPE_BITS)))   \
                                  | (mode << ((bit) * GPIO_OUTPUT_TYPE_BITS)); }



// GPIO control
#define	GPIO_BIT_CLR(p,bit)      p->BSRR = ((1U<<(bit)) << 16)
#define	GPIO_BIT_SET(p,bit)      p->BSRR = (1U<<(bit))

#define	GPIO_IN(p,bit)	(p->IDR & (1<<(bit)))

//	GPIOx->IDR == input data register
//	GPIOx->ODR == output data register

#define	GPIO_AF_USART1 (7)
#define	GPIO_AF_USART2 (7)
#define	GPIO_AF_USART3 (7)
#define	GPIO_AF_UART4  (5)

#define	GPIO_AF_SPI    (5)
// #define	GPIO_AF_SPI1   (5)
// #define	GPIO_AF_SPI2   (5)

extern void gpio_init (void);
extern void gpio_rele_off (void);
extern void gpio_rele_on (void);

#endif // ~GPIO_H

