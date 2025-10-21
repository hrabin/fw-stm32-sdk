# to be included in Makefile

ifndef DIR_SDK
  $(error DIR_SDK not set)
endif

DIR_STM := $(DIR_SDK)/stm32
DIR_DRV := $(DIR_SDK)/drv_g4
DIR_HAL := $(DIR_SDK)/hal
DIR_SYS := $(DIR_STM)/CMSIS
DIR_COMMON := $(DIR_SDK)/common

PREFIX = @echo $@;arm-none-eabi-

ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif

HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

CPU = -mcpu=cortex-m4
FPU = -mfpu=fpv4-sp-d16
FLOAT-ABI = -mfloat-abi=hard
MCU = $(CPU) -mthumb  $(FPU) $(FLOAT-ABI)

STM32_HAL = $(DIR_STM)/STM32G4xx_HAL_Driver

C_SOURCES =  \
  $(DIR_SYS)/src/system_stm32g4xx.c \
  $(STM32_HAL)/Src/stm32g4xx_ll_adc.c \
  $(STM32_HAL)/Src/stm32g4xx_ll_dma.c \
  $(STM32_HAL)/Src/stm32g4xx_ll_exti.c \
  $(STM32_HAL)/Src/stm32g4xx_ll_gpio.c \
  $(STM32_HAL)/Src/stm32g4xx_ll_pwr.c \
  $(STM32_HAL)/Src/stm32g4xx_ll_rcc.c \
  $(STM32_HAL)/Src/stm32g4xx_ll_rtc.c \
  $(STM32_HAL)/Src/stm32g4xx_ll_spi.c \
  $(STM32_HAL)/Src/stm32g4xx_ll_tim.c \
  $(STM32_HAL)/Src/stm32g4xx_ll_utils.c \
  $(STM32_HAL)/Src/stm32g4xx_hal_cortex.c \
  $(STM32_HAL)/Src/stm32g4xx_hal_pcd.c \
  $(STM32_HAL)/Src/stm32g4xx_hal_pcd_ex.c \

ASM_SOURCES =  \
  $(DIR_SYS)/src/startup_stm32g473xx.s

AS_DEFS = 

C_DEFS =  \
-DUSE_FULL_LL_DRIVER \
-DHSE_VALUE=16000000 \
-DLSE_VALUE=32768 \
-DHSI_VALUE=8000000 \
-DLSI_VALUE=40000 \
-DVDD_VALUE=3300 \
-DSTM32G473xx

C_INCLUDES =  \
-I$(DIR_COMMON) \
-I$(DIR_DRV) \
-I$(DIR_HAL) \
-I$(DIR_SDK) \
-I$(DIR_SYS) \
-I$(DIR_STM) \
-I$(STM32_HAL)/Inc \
-I$(DIR_SYS)/device/STM32G4xx/inc \
-I$(DIR_SYS)/inc

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

ifeq ($(BOOT), 1)
LDSCRIPT = $(DIR_SYS)/linker/STM32G473RCTx-boot.ld
else
LDSCRIPT = $(DIR_SYS)/linker/STM32G473RCTx.ld
endif


LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -specs=nosys.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections,--print-memory-usage

