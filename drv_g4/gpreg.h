#ifndef GPREG_H
#define	GPREG_H

#define GPREG_BOOT_POR           0x00000000UL // default
#define	GPREG_BOOT_REBOOT        0x00000100UL // 
#define GPREG_BOOT_FLASH_RQ      0xABABAB00UL // request to flash FW from external storage
#define GPREG_BOOT_STAY_IN_BOOT  0xAAABBB00UL // request to stay in bootloader

#define GPREG_WDID_POR           0x00000000UL
#define GPREG_WDID_ERROR         0x12345678UL
#define GPREG_WDID_REBOOT_RQ     0x2d78b098UL

#define GPREG_INFO_TASK_DEAD     0x10000000UL

#define GPREG_RD_ENABLE  // {RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;}
#define GPREG_WR_ENABLE  {PWR->CR1 |= PWR_CR1_DBP;}
#define GPREG_WR_DISABLE {PWR->CR1 &= ~PWR_CR1_DBP;}

// used registers
#define GPREG_BOOT     TAMP->BKP10R // app / bootloader communication
#define GPREG_WDID     TAMP->BKP11R // watchdog-reset reason info
#define GPREG_INFO     TAMP->BKP12R // suppplement info for watchdog reset
#define GPREG_BOOTLOOP TAMP->BKP13R // boot-loop detection counter

#define	GPREG_WRITE(reg, value) {GPREG_WR_ENABLE; reg=value;}

#endif // ! GPREG_H

