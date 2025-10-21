#include "os.h"
#include "hardware.h"
#include "time.h"

#include "spi.h"

#include "lis2dh12.h"

#ifdef HW_ACCL_SPI_INIT

 #define SPI_TRANSFER(value) HW_ACCL_SPI_TRANSFER(value)
 #define SPI_INIT()  {   \
	 HW_ACCL_SPI_INIT(); \
	 HW_ACCL_CS_INIT;    \
	 HW_ACCL_CS_HI;      \
 }
 // SPI mode CPOL0 CPHA1

#else //not HW_ACCL_SPI_INIT

 #warning "SHOCK USES SW SPI"
 static u8 SPI_TRANSFER(u8 value);
 
 #define SPI_INIT() { \
	HW_ACCL_SCK_INIT; \
	HW_ACCL_MISO_INIT; \
	HW_ACCL_MOSI_INIT; \
 }

#endif // end not SSP1_USED

#define	_ADDR_READ  (1<<7)
#define	_ADDR_INC   (1<<6)

#define	LIS2DH12_REG_WHO_AM_I (0x0F)
#define	 _WHO_AM_I_VALUE (0x33)


#define LIS2DH12_REG_CTRL1 (0x20)
#define     _CTRL1_XEN       (1 << 0)
#define     _CTRL1_YEN       (1 << 1)
#define     _CTRL1_ZEN       (1 << 2)
#define     _CTRL1_LPEN      (1 << 3)
#define     _CTRL1_ODR_OFF   (0 << 4)
#define     _CTRL1_ODR_1HZ   (1 << 4)
#define     _CTRL1_ODR_10HZ  (2 << 4)
#define     _CTRL1_ODR_25HZ  (3 << 4)
#define     _CTRL1_ODR_50HZ  (4 << 4)
#define     _CTRL1_ODR_100HZ (5 << 4)
#define     _CTRL1_ODR_200HZ (6 << 4)
#define     _CTRL1_ODR_400HZ (7 << 4)

#define LIS2DH12_REG_CTRL2 (0x21)
#define     _CTRL2_HP_IA1    (1 << 0) // High-pass filter enable for INT1
#define     _CTRL2_HP_IA2    (1 << 1) // High-pass filter enable for INT2
#define     _CTRL2_FDS       (1 << 3) // Filtered data selection (relative instead of absolute data)
#define     _CTRL2_HP_CLICK  (1 << 2) // High-pass filter enabled for CLICK function

#define LIS2DH12_REG_CTRL3 (0x22)
#define     _CTRL3_I1_IA2    (1 << 5)
#define     _CTRL3_I1_IA1    (1 << 6)

#define LIS2DH12_REG_CTRL4 (0x23)
#define     _CTRL4_HR        (1 << 3) // High Resolution
#define     _CTRL4_BDU       (1 << 7) // Block data update. 0: continuous; 1: output regs not updated until MSB and LSB have been read

#define LIS2DH12_REG_CTRL5 (0x24)
#define     _CTRL5_BOOT      (1 << 7) // reboot memory content (reset)
#define     _CTRL5_LIR_INT1  (1 << 3) // Latch interrupt request, cleared by reading INT1_SRC (31h)
#define     _CTRL5_LIR_INT2  (1 << 1) // Latch interrupt request, cleared by reading INT2_SRC (35h)

#define LIS2DH12_REG_CTRL6 (0x25)
#define     _CTRL6_INT_POLARITY (1 << 1) // interrupt pins polarity (default 0 == active high)
#define     _CTRL6_I2_ACT       (1 << 3) // activity on INT2
#define     _CTRL6_I2_BOOT      (1 << 4) // boot on INT2
#define     _CTRL6_I2_IA2       (1 << 5) // interrupt 2 function on INT2
#define     _CTRL6_I2_IA1       (1 << 6) // interrupt 1 function on INT2
#define     _CTRL6_I2_CLICK     (1 << 7) // click interrupt on INT2

#define LIS2DH12_REG_REF (0x26)

#define LIS2DH12_REG_STATUS (0x27)

#define LIS2DH12_REG_OUT_X_L (0x28)
#define LIS2DH12_REG_OUT_X_H (0x29)
#define LIS2DH12_REG_OUT_Y_L (0x2A)
#define LIS2DH12_REG_OUT_Y_H (0x2B)
#define LIS2DH12_REG_OUT_Z_L (0x2C)
#define LIS2DH12_REG_OUT_Z_H (0x2D)

#define LIS2DH12_REG_INT1_CFG (0x30)
#define     _INT1_CFG_XLIE (1 << 0)
#define     _INT1_CFG_XHIE (1 << 1)
#define     _INT1_CFG_YLIE (1 << 2)
#define     _INT1_CFG_YHIE (1 << 3)
#define     _INT1_CFG_ZLIE (1 << 4)
#define     _INT1_CFG_ZHIE (1 << 5)
#define     _INT1_CFG_6D   (1 << 6)
#define     _INT1_CFG_AOI  (1 << 7)

#define LIS2DH12_REG_INT1_SRC (0x31)

#define LIS2DH12_REG_INT1_THS (0x32)

#define LIS2DH12_REG_INT1_DURATION (0x33)

#define LIS2DH12_REG_INT2_CFG (0x34)
#define     _INT2_CFG_XLIE (1 << 0)
#define     _INT2_CFG_XHIE (1 << 1)
#define     _INT2_CFG_YLIE (1 << 2)
#define     _INT2_CFG_YHIE (1 << 3)
#define     _INT2_CFG_ZLIE (1 << 4)
#define     _INT2_CFG_ZHIE (1 << 5)
#define     _INT2_CFG_6D   (1 << 6)
#define     _INT2_CFG_AOI  (1 << 7)

#define LIS2DH12_REG_INT2_SRC (0x35)

#define LIS2DH12_REG_INT2_THS (0x36)

#define LIS2DH12_REG_INT2_DURATION (0x37)


static u8 _reg_read(const u8 reg)
{
	u8 value;

	HW_ACCL_CS_LOW;
	SPI_TRANSFER(_ADDR_READ | reg);
	value = SPI_TRANSFER(0);
	HW_ACCL_CS_HI;

	return (value);
}

static void _reg_write(const u8 reg, u8 value)
{
	HW_ACCL_CS_LOW;
	SPI_TRANSFER(reg);
	SPI_TRANSFER(value);
	HW_ACCL_CS_HI;
}



bool lis2dh12_init(void)
{
	HW_ACCL_CS_INIT; 
	HW_ACCL_CS_HI;

	SPI_INIT();

	return (true);
}

bool lis2dh12_set_sensitivity(u8 threshold, u8 duration)
{
	_reg_write(LIS2DH12_REG_INT1_THS, threshold);
    _reg_write(LIS2DH12_REG_INT1_DURATION, duration);
     _reg_read(LIS2DH12_REG_REF);

    _reg_write(LIS2DH12_REG_INT1_CFG, (threshold && duration) ? _INT1_CFG_XHIE | _INT1_CFG_YHIE | _INT1_CFG_ZHIE : 0);
	return (true);
}

bool lis2dh12_start(void)
{
	HW_ACCL_CS_HI;
    OS_DELAY(10);
	if (_reg_read(LIS2DH12_REG_WHO_AM_I) != _WHO_AM_I_VALUE)
	{
		return (false);
	}
	OS_PRINTF("[LIS2DH12] ");

    _reg_write(LIS2DH12_REG_CTRL5, _CTRL5_BOOT); // reboot
    OS_DELAY(10);
    _reg_write(LIS2DH12_REG_CTRL1, _CTRL1_XEN | _CTRL1_YEN | _CTRL1_ZEN | _CTRL1_ODR_100HZ);
    // _reg_write(LIS2DH12_REG_CTRL2, _CTRL2_HP_IA1 | _CTRL2_HP_IA2 );
    // _reg_write(LIS2DH12_REG_CTRL3, _CTRL3_I1_IA1);
    _reg_write(LIS2DH12_REG_CTRL4, _CTRL4_BDU | _CTRL4_HR);
    _reg_write(LIS2DH12_REG_CTRL5, _CTRL5_LIR_INT1);
    // _reg_write(LIS2DH12_REG_CTRL6, _CTRL6_I2_IA2);

    lis2dh12_set_sensitivity(0, 0);

	return (true);
}


bool lis2dh12_read_data(s16 *x, s16 *y, s16 *z)
{
	*x = 0;
	*y = 0;
	*z = 0;

	HW_ACCL_CS_LOW;

	SPI_TRANSFER(_ADDR_INC | _ADDR_READ | LIS2DH12_REG_OUT_X_L);

	*x  = SPI_TRANSFER(0);
	*x |= SPI_TRANSFER(0) << 8;
	*y  = SPI_TRANSFER(0);
	*y |= SPI_TRANSFER(0) << 8;
	*z  = SPI_TRANSFER(0);
	*z |= SPI_TRANSFER(0) << 8;

	HW_ACCL_CS_HI;

	// LIS2DH12_REG_INT1_SRC ?
	return (true);
}

#ifndef HW_ACCL_SPI_INIT  // in case HW SPI not used 

	static u8 SPI_TRANSFER(u8 value)
	{
		u8 bitcnt;
		u8 retval;
		for (bitcnt = 0; bitcnt < 8; bitcnt++)
		{
			if (value & 0x80)
				ACCL_MOSI_HI;
			else
				ACCL_MOSI_LO;
			value <<= 1;
			ACCL_SCK_HI;
			retval <<= 1;
			if (ACCL_MISO_IN)
				retval |= 1;
			ACCL_SCK_LO;
		}
		ACCL_MOSI_LO;
		return (retval);
	}	

#endif // not HW_ACCL_SPI_INIT 

