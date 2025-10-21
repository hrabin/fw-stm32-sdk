#ifndef UART_H
#define UART_H

#include "type.h"
#include "hardware.h"

#if UART1_ON
	#define UART1_IRQ_USED 			1
  #ifndef UART1_RX_BUF_DEPTH
	#define UART1_RX_BUF_DEPTH 128
  #endif // not defined UART1_RX_BUF_DEPTH
  #ifndef UART1_TX_BUF_DEPTH
	#define UART1_TX_BUF_DEPTH 523
  #endif // not defined UART1_TX_BUF_DEPTH

	int uart1_putchar(const char c);
	int uart1_getchar(void);
	void uart1_init(u32 baudrate);	
	bool uart1_idle(void);	
#endif // UART1_ON

#if UART2_ON
	#define UART2_IRQ_USED 			1
  #ifndef UART2_TX_BUF_DEPTH
	#define UART2_TX_BUF_DEPTH 128
  #endif // not defined UART2_TX_BUF_DEPTH
  #ifndef UART2_RX_BUF_DEPTH
	#define UART2_RX_BUF_DEPTH 256
  #endif // not defined UART2_RX_BUF_DEPTH

	int uart2_putchar(const char c);
	int uart2_getchar(void);
	void uart2_init(u32 baudrate);	
	void uart2_sleep(void);
	void uart2_wakeup(void);
#endif // UART2_ON

#if UART3_ON
	#define UART3_IRQ_USED 			1
  #ifndef UART3_RX_BUF_DEPTH
	#define UART3_RX_BUF_DEPTH 256
  #endif // not defined UART3_RX_BUF_DEPTH
  #ifndef UART3_TX_BUF_DEPTH
	#define UART3_TX_BUF_DEPTH 256
  #endif // not defined UART3_TX_BUF_DEPTH

	int uart3_putchar(const char c);
	int uart3_getchar(void);
	void uart3_init(u32 baudrate);	
#endif // UART3_ON

#if UART4_ON
	#define UART4_IRQ_USED 			1
  #ifndef UART4_RX_BUF_DEPTH
	#define UART4_RX_BUF_DEPTH 256
  #endif // not defined UART4_RX_BUF_DEPTH
  #ifndef UART4_TX_BUF_DEPTH
	#define UART4_TX_BUF_DEPTH 256
  #endif // not defined UART4_TX_BUF_DEPTH

	int uart4_putchar(const char c);
	int uart4_getchar(void);
	void uart4_init(u32 baudrate);	
#endif // UART4_ON

#endif // UART_H
