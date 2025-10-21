#include "platform_setup.h"
#include "hardware.h"
#include "uart.h"
#include "irq.h"
#include "gpio.h"

#include "stm32g4xx_ll_usart.h"

u32 uart_brr_value(u32 clk, u32 baudrate)
{	// return BRR register value for selected baudrate
	return (__LL_USART_DIV_SAMPLING16(clk, LL_USART_PRESCALER_DIV1, baudrate));
}

#if UART1_ON

static char u1_rx_buf[UART1_RX_BUF_DEPTH];
static volatile u32	u1_rx_wr_index;
static volatile u32	u1_rx_wr_cnt;
static volatile u32 u1_rx_rd_index;
static volatile u32 u1_rx_rd_cnt;

static void _u1_rx(const char c)
{
	if ((u32)(u1_rx_wr_cnt-u1_rx_rd_cnt) >= UART1_RX_BUF_DEPTH)
	{	//buffer full
		return;
	}

	u1_rx_buf[u1_rx_wr_index]=c;

	if (++u1_rx_wr_index >= UART1_RX_BUF_DEPTH)
		u1_rx_wr_index=0;

	u1_rx_wr_cnt++;
}

int uart1_getchar(void)
{
	int c;

	if(u1_rx_rd_cnt == u1_rx_wr_cnt)
	{	//buffer empty
		return (-1);
	}

	c=u1_rx_buf[u1_rx_rd_index];

	if (++u1_rx_rd_index >= UART1_RX_BUF_DEPTH)
		u1_rx_rd_index=0;

	u1_rx_rd_cnt++;
	return (c);
}

static char u1_tx_buf[UART1_TX_BUF_DEPTH];
static volatile u32 u1_tx_wr_index;
static volatile u32 u1_tx_wr_cnt;
static volatile u32 u1_tx_rd_index;
static volatile u32 u1_tx_rd_cnt;
static volatile int u1_tx_run;

int uart1_putchar(const char c)
{
	if ((u32)(u1_tx_wr_cnt-u1_tx_rd_cnt) >= UART1_TX_BUF_DEPTH)
	{	//buffer full
		return (false);
	}
	u1_tx_buf[u1_tx_wr_index] = c;

	if (++u1_tx_wr_index >= UART1_TX_BUF_DEPTH)
		u1_tx_wr_index = 0;
	u1_tx_wr_cnt++;

	if (u1_tx_run == false)
	{
		u1_tx_run = true;
		USART1->CR1 |= USART_CR1_TXEIE; 
		// UART1
	}
	return (true);
}

static void _u1_tx(void)
{
	char c;
	c=u1_tx_buf[u1_tx_rd_index];

	if (++u1_tx_rd_index >= UART1_TX_BUF_DEPTH)
		u1_tx_rd_index=0;

	u1_tx_rd_cnt++;
	USART1->TDR = c;
}

#if UART1_IRQ_USED
void USART1_IRQHandler(void)
{
	u16 status;

	status=USART1->ISR;

	if((status & USART_ISR_TXE) && (USART1->CR1 & USART_CR1_TXEIE))
	{
		if (u1_tx_wr_cnt == u1_tx_rd_cnt)//buffer empty
		{
			USART1->CR1 &= ~USART_CR1_TXEIE;
			u1_tx_run=false;
		}
		else
		{
			_u1_tx();
		}
	}
	if(status & USART_ISR_RXNE)
	{
		_u1_rx(USART1->RDR);	
	}
}
#endif // UART1_IRQ_USED

void uart1_init(u32 baudrate)
{
	// enable GPIO clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN;
	// enable USART1 clock
  	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	// RX: PC5, TX: PC4
#define _U1_RX_PORT GPIOC
#define _U1_RX_BIT 5
#define _U1_TX_PORT GPIOC
#define _U1_TX_BIT 4

	GPIO_AFR(_U1_RX_PORT, _U1_RX_BIT, GPIO_AF_USART1);
	GPIO_AFR(_U1_TX_PORT, _U1_TX_BIT, GPIO_AF_USART1);

	GPIO_MODE(_U1_RX_PORT, _U1_RX_BIT, GPIO_MODE_ALT)
	GPIO_MODE(_U1_TX_PORT, _U1_TX_BIT, GPIO_MODE_ALT)

	GPIO_SPEED(_U1_RX_PORT, _U1_RX_BIT, GPIO_SPEED_LOW);
	GPIO_SPEED(_U1_TX_PORT, _U1_TX_BIT, GPIO_SPEED_LOW);

	// GPIO_PUPDN(GPIOE, 1, GPIO_PUPDN_UP);

	USART1->BRR = uart_brr_value(UART1CLK, baudrate);
	// HAL_UART_Init

#if UART1_IRQ_USED
  	irq_enable(USART1_IRQn, IRQ_PRIO_UART1);
#endif

	USART1->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_UE | USART_CR1_TXEIE | USART_CR1_RXNEIE;
}

bool uart1_idle(void)
{
	if (u1_tx_wr_cnt == u1_tx_rd_cnt)
		return (true);

	return (false);
}
#endif //UART1_ON

#if UART2_ON

static char u2_rx_buf[UART2_RX_BUF_DEPTH];
static volatile u32	u2_rx_wr_index;
static volatile u32	u2_rx_wr_cnt;
static volatile u32 u2_rx_rd_index;
static volatile u32 u2_rx_rd_cnt;

static void _u2_rx(const char c)
{
	if ((u32)(u2_rx_wr_cnt-u2_rx_rd_cnt) >= UART2_RX_BUF_DEPTH)
	{	//buffer full
		return;
	}

	u2_rx_buf[u2_rx_wr_index]=c;

	if (++u2_rx_wr_index >= UART2_RX_BUF_DEPTH)
		u2_rx_wr_index=0;

	u2_rx_wr_cnt++;
}

int uart2_getchar(void)
{
	int c;

	if(u2_rx_rd_cnt == u2_rx_wr_cnt)
	{	//buffer empty
		return (-1);
	}

	c=u2_rx_buf[u2_rx_rd_index];

	if (++u2_rx_rd_index >= UART2_RX_BUF_DEPTH)
		u2_rx_rd_index=0;

	u2_rx_rd_cnt++;
	return (c);
}

static char u2_tx_buf[UART2_TX_BUF_DEPTH];
static volatile u32 u2_tx_wr_index;
static volatile u32 u2_tx_wr_cnt;
static volatile u32 u2_tx_rd_index;
static volatile u32 u2_tx_rd_cnt;
static volatile int u2_tx_run;

int uart2_putchar(const char c)
{
	if ((u32)(u2_tx_wr_cnt-u2_tx_rd_cnt) >= UART2_TX_BUF_DEPTH)
	{	//buffer full
		return (false);
	}
	u2_tx_buf[u2_tx_wr_index] = c;

	if (++u2_tx_wr_index >= UART2_TX_BUF_DEPTH)
		u2_tx_wr_index = 0;
	u2_tx_wr_cnt++;

	if (u2_tx_run == false)
	{
		u2_tx_run = true;
		USART2->CR1 |= USART_CR1_TXEIE; 
		// UART2
	}
	return (true);
}

static void _u2_tx(void)
{
	char c;
	c=u2_tx_buf[u2_tx_rd_index];

	if (++u2_tx_rd_index >= UART2_TX_BUF_DEPTH)
		u2_tx_rd_index=0;

	u2_tx_rd_cnt++;
	USART2->TDR = c;
}

#if UART2_IRQ_USED
void USART2_IRQHandler(void)
{
	u16 status;

	status=USART2->ISR;

	if((status & USART_ISR_TXE) && (USART2->CR1 & USART_CR1_TXEIE))
	{
		if(u2_tx_wr_cnt == u2_tx_rd_cnt)//buffer empty
		{
			USART2->CR1 &= ~USART_CR1_TXEIE;
			u2_tx_run=false;
		}
		else
		{
			_u2_tx();
		}
	}
	if(status & USART_ISR_RXNE)
	{
		_u2_rx(USART2->RDR);	
	}
}
#endif // UART2_IRQ_USED

void uart2_init(u32 baudrate)
{
	// enable GPIO clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIODEN;
	// enable USART2 clock
  	RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
	// RX: PA15, TX: PB3
#define _U2_RX_PORT GPIOA
#define _U2_RX_BIT 15
#define _U2_TX_PORT GPIOB
#define _U2_TX_BIT 3

	GPIO_AFR(_U2_RX_PORT, _U2_RX_BIT, GPIO_AF_USART2);
	GPIO_AFR(_U2_TX_PORT, _U2_TX_BIT, GPIO_AF_USART2);

	GPIO_MODE(_U2_RX_PORT, _U2_RX_BIT, GPIO_MODE_ALT)
	GPIO_MODE(_U2_TX_PORT, _U2_TX_BIT, GPIO_MODE_ALT)

	GPIO_SPEED(_U2_RX_PORT, _U2_RX_BIT, GPIO_SPEED_LOW);
	GPIO_SPEED(_U2_TX_PORT, _U2_TX_BIT, GPIO_SPEED_LOW);
	
	// GPIO_PUPDN(GPIOE, 1, GPIO_PUPDN_UP);

	USART2->BRR = uart_brr_value(UART2CLK, baudrate);
	// HAL_UART_Init

#if UART2_IRQ_USED
  	irq_enable(USART2_IRQn, IRQ_PRIO_UART2);
#endif

	USART2->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_UE | USART_CR1_TXEIE | USART_CR1_RXNEIE;
}

void uart2_sleep(void)
{
	GPIO_MODE(_U2_TX_PORT, _U2_TX_BIT, GPIO_MODE_INPUT)
  	RCC->APB1ENR1 &= ~RCC_APB1ENR1_USART2EN;
}

void uart2_wakeup(void)
{
	GPIO_MODE(_U2_TX_PORT, _U2_TX_BIT, GPIO_MODE_ALT)
  	RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
}

#endif // UART2_ON

#if UART3_ON

static char u3_rx_buf[UART3_RX_BUF_DEPTH];
static volatile u32	u3_rx_wr_index;
static volatile u32	u3_rx_wr_cnt;
static volatile u32 u3_rx_rd_index;
static volatile u32 u3_rx_rd_cnt;

static void _u3_rx(const char c)
{
	if ((u32)(u3_rx_wr_cnt-u3_rx_rd_cnt) >= UART3_RX_BUF_DEPTH)
	{	//buffer full
		return;
	}

	u3_rx_buf[u3_rx_wr_index]=c;

	if (++u3_rx_wr_index >= UART3_RX_BUF_DEPTH)
		u3_rx_wr_index=0;

	u3_rx_wr_cnt++;
}

int uart3_getchar(void)
{
	int c;

	if(u3_rx_rd_cnt == u3_rx_wr_cnt)
	{	//buffer empty
		return (-1);
	}

	c=u3_rx_buf[u3_rx_rd_index];

	if (++u3_rx_rd_index >= UART3_RX_BUF_DEPTH)
		u3_rx_rd_index=0;

	u3_rx_rd_cnt++;
	return (c);
}

static char u3_tx_buf[UART3_TX_BUF_DEPTH];
static volatile u32 u3_tx_wr_index;
static volatile u32 u3_tx_wr_cnt;
static volatile u32 u3_tx_rd_index;
static volatile u32 u3_tx_rd_cnt;
static volatile int u3_tx_run;

int uart3_putchar(const char c)
{
	if ((u32)(u3_tx_wr_cnt-u3_tx_rd_cnt) >= UART3_TX_BUF_DEPTH)
	{	//buffer full
		return (false);
	}
	u3_tx_buf[u3_tx_wr_index] = c;

	if (++u3_tx_wr_index >= UART3_TX_BUF_DEPTH)
		u3_tx_wr_index = 0;
	u3_tx_wr_cnt++;

	if (u3_tx_run == false)
	{
		u3_tx_run = true;
		USART3->CR1 |= USART_CR1_TXEIE; 
		// UART3
	}
	return (true);
}

static void _u3_tx(void)
{
	char c;
	c=u3_tx_buf[u3_tx_rd_index];

	if (++u3_tx_rd_index >= UART3_TX_BUF_DEPTH)
		u3_tx_rd_index=0;

	u3_tx_rd_cnt++;
	USART3->TDR = c;
}

#if UART3_IRQ_USED
void USART3_IRQHandler(void)
{
	u16 status;

	status=USART3->ISR;

	if((status & USART_ISR_TXE) && (USART3->CR1 & USART_CR1_TXEIE))
	{
		if(u3_tx_wr_cnt == u3_tx_rd_cnt)//buffer empty
		{
			USART3->CR1 &= ~USART_CR1_TXEIE;
			u3_tx_run=false;
		}
		else
		{
			_u3_tx();
		}
	}
	if(status & USART_ISR_RXNE)
	{
		_u3_rx(USART3->RDR);	
	}
}
#endif // UART3_IRQ_USED

void uart3_init(u32 baudrate)
{
	// enable GPIO clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIODEN;
	// enable USART3 clock
  	RCC->APB1ENR1 |= RCC_APB1ENR1_USART3EN;

	// RX: PB11, TX: PB10
#define _U3_RX_PORT GPIOB
#define _U3_RX_BIT 11
#define _U3_TX_PORT GPIOB
#define _U3_TX_BIT 10

	GPIO_AFR(_U3_RX_PORT, _U3_RX_BIT, GPIO_AF_USART3);
	GPIO_AFR(_U3_TX_PORT, _U3_TX_BIT, GPIO_AF_USART3);

	GPIO_MODE(_U3_RX_PORT, _U3_RX_BIT, GPIO_MODE_ALT)
	GPIO_MODE(_U3_TX_PORT, _U3_TX_BIT, GPIO_MODE_ALT)

	GPIO_SPEED(_U3_RX_PORT, _U3_RX_BIT, GPIO_SPEED_LOW);
	GPIO_SPEED(_U3_TX_PORT, _U3_TX_BIT, GPIO_SPEED_LOW);

	USART3->BRR = uart_brr_value(UART3CLK, baudrate);

#if UART3_IRQ_USED
  	irq_enable(USART3_IRQn, IRQ_PRIO_UART3);
#endif

	USART3->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_UE | USART_CR1_TXEIE | USART_CR1_RXNEIE;
}
#endif // UART3_ON

#if UART4_ON

static char u4_rx_buf[UART4_RX_BUF_DEPTH];
static volatile u32	u4_rx_wr_index;
static volatile u32	u4_rx_wr_cnt;
static volatile u32 u4_rx_rd_index;
static volatile u32 u4_rx_rd_cnt;

static void _u4_rx(const char c)
{
	if ((u32)(u4_rx_wr_cnt-u4_rx_rd_cnt) >= UART4_RX_BUF_DEPTH)
	{	//buffer full
		return;
	}

	u4_rx_buf[u4_rx_wr_index]=c;

	if (++u4_rx_wr_index >= UART4_RX_BUF_DEPTH)
		u4_rx_wr_index=0;

	u4_rx_wr_cnt++;
}

int uart4_getchar(void)
{
	int c;

	if(u4_rx_rd_cnt == u4_rx_wr_cnt)
	{	//buffer empty
		return (-1);
	}

	c=u4_rx_buf[u4_rx_rd_index];

	if (++u4_rx_rd_index >= UART4_RX_BUF_DEPTH)
		u4_rx_rd_index=0;

	u4_rx_rd_cnt++;
	return (c);
}

static char u4_tx_buf[UART4_TX_BUF_DEPTH];
static volatile u32 u4_tx_wr_index;
static volatile u32 u4_tx_wr_cnt;
static volatile u32 u4_tx_rd_index;
static volatile u32 u4_tx_rd_cnt;
static volatile int u4_tx_run;

int uart4_putchar(const char c)
{
	if ((u32)(u4_tx_wr_cnt-u4_tx_rd_cnt) >= UART4_TX_BUF_DEPTH)
	{	//buffer full
		return (false);
	}
	u4_tx_buf[u4_tx_wr_index] = c;

	if (++u4_tx_wr_index >= UART4_TX_BUF_DEPTH)
		u4_tx_wr_index = 0;
	u4_tx_wr_cnt++;

	if (u4_tx_run == false)
	{
		u4_tx_run = true;
		UART4->CR1 |= USART_CR1_TXEIE; 
		// UART4
	}
	return (true);
}

static void _u4_tx(void)
{
	char c;
	c=u4_tx_buf[u4_tx_rd_index];

	if (++u4_tx_rd_index >= UART4_TX_BUF_DEPTH)
		u4_tx_rd_index=0;

	u4_tx_rd_cnt++;
	UART4->TDR = c;
}

#if UART4_IRQ_USED
void UART4_IRQHandler(void)
{
	u16 status;

	status=UART4->ISR;

	if((status & USART_ISR_TXE) && (UART4->CR1 & USART_CR1_TXEIE))
	{
		if(u4_tx_wr_cnt == u4_tx_rd_cnt)//buffer empty
		{
			UART4->CR1 &= ~USART_CR1_TXEIE;
			u4_tx_run=false;
		}
		else
		{
			_u4_tx();
		}
	}
	if(status & USART_ISR_RXNE)
	{
		_u4_rx(UART4->RDR);	
	}
}
#endif // UART4_IRQ_USED

void MemManage_Handler(void)
{	// WTF is this ?
	// instead of UART4_IRQHandler() it call MemManage_Handler() ? 
	// How to fix it ? It looks like HW error ....
	// Interresting is: MemManage_Handler + 64 == UART4_IRQHandler
	UART4_IRQHandler();
}


void uart4_init(u32 baudrate)
{
	// enable GPIO clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
	// enable UART4 clock
  	RCC->APB1ENR1 |= RCC_APB1ENR1_UART4EN;

	// RX: PB11, TX: PB10
#define _U4_RX_PORT GPIOC
#define _U4_RX_BIT 11
#define _U4_TX_PORT GPIOC
#define _U4_TX_BIT 10

	GPIO_AFR(_U4_RX_PORT, _U4_RX_BIT, GPIO_AF_UART4);
	GPIO_AFR(_U4_TX_PORT, _U4_TX_BIT, GPIO_AF_UART4);

	GPIO_MODE(_U4_RX_PORT, _U4_RX_BIT, GPIO_MODE_ALT)
	GPIO_MODE(_U4_TX_PORT, _U4_TX_BIT, GPIO_MODE_ALT)

	GPIO_SPEED(_U4_RX_PORT, _U4_RX_BIT, GPIO_SPEED_LOW);
	GPIO_SPEED(_U4_TX_PORT, _U4_TX_BIT, GPIO_SPEED_LOW);
	
	GPIO_PUPDN(_U4_RX_PORT, _U4_RX_BIT, GPIO_PUPDN_UP);

	UART4->BRR = uart_brr_value(UART4CLK, baudrate);

 	UART4->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_UE | USART_CR1_TXEIE | USART_CR1_RXNEIE;

#if UART4_IRQ_USED
  	irq_enable(UART4_IRQn, IRQ_PRIO_UART4);
#endif
}
#endif // UART4_ON
