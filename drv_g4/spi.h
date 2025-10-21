#ifndef _SPI_H
#define _SPI_H

#include "hardware.h"

// SPIx_ON must be set in pcb_xxxxxxx.h 

#ifndef SPI1_ON
  #define	SPI1_ON 0
#endif

#ifndef SPI3_ON
  #define	SPI3_ON 0
#endif

#ifndef SPI5_ON
  #define	SPI5_ON 0
#endif


#if SPI1_ON 

	void spi1_init (void);
	unsigned char spi1_transfer(unsigned char c);

#endif // SPI1_ON

#if SPI3_ON 

	void spi3_init (void);
	unsigned char spi3_transfer(unsigned char c);

#endif // SPI3_ON

#if SPI5_ON 

	void spi5_init (void);
	unsigned char spi5_transfer(unsigned char c);

#endif // SPI1_ON

#endif //_SPI_H

