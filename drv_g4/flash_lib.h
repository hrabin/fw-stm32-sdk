#ifndef FLASH_LIB_H
#define	FLASH_LIB_H

#include "type.h"

bool flash_init (void);
u32 flash_get_size(void);
u8 flash_read_status (void);

bool flash_data_erase (u32 addr, u32 len);
bool flash_read_data (u8 *dest, u32 addr, u32 len);
bool flash_write_data (u32 addr, u8 *src, u32 len);
bool flash_write_data_noerase (u32 addr, u8 *src, u16 len);
bool flash_erase_sector (u32 addr);
bool flash_content_changed(void);
void flash_flush_cache (void);
void flash_maintenace_task (void);

#endif // ! FLASH_LIB_H
