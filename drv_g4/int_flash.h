#ifndef INT_FLASH_H
#define INT_FLASH_H

typedef enum
{ 
  INT_FLASH_OK,
  INT_FLASH_BUSY = 1,
  INT_FLASH_ERROR_WRP,
  INT_FLASH_ERROR_PROGRAM,
} flash_status_e;

bool ifl_init(void);
flash_status_e ifl_get_status(void);
flash_status_e ifl_wait_for_last_operation(void);
bool int_flash_write(u32 addr, u32 *data, u32 len);

#endif // ! INT_FLASH_H
