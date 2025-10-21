#include "common.h"
#include "int_flash.h"
#include "wd.h"
#include "hardware.h"

#include "log.h"

LOG_DEF("IFL");

#define FLASH_KEY1               ((u32)0x45670123)
#define FLASH_KEY2               ((u32)0xCDEF89AB)

#define FLASH_OPT_KEY1           ((u32)0x08192A3B)
#define FLASH_OPT_KEY2           ((u32)0x4C5D6E7F)

static int begin_of_page (u32 addr)
{	
	if (addr < FLASH_START_ADDR)
		return (-1); // not flash address

	if (addr & (FLASH_PAGE_SIZE-1))
		return (-2); // not begin of sector (2k)

	addr -= FLASH_START_ADDR; //  0x08000000
	return (addr>>FLASH_PAGE_BITS);
}

static void flash_unlock(void)
{
	if((FLASH->CR & FLASH_CR_LOCK))
	{
		// Authorize the FLASH Registers access
		FLASH->KEYR = FLASH_KEY1;
		FLASH->KEYR = FLASH_KEY2;
	}
}

static void flash_lock(void)
{
	// Set the LOCK Bit to lock the FLASH Registers access
	FLASH->CR |= FLASH_CR_LOCK;
}

bool ifl_init(void)
{
	/*FLASH->OPTKEYR = FLASH_OPT_KEY1;
	FLASH->OPTKEYR = FLASH_OPT_KEY2;
	FLASH->OPTR &= ~FLASH_OPTR_DBANK;*/
	return (true);
}

flash_status_e fl_getstatus(void)
{
	flash_status_e flashstatus = INT_FLASH_OK;
	
	if ((FLASH->SR & FLASH_SR_BSY) == FLASH_SR_BSY) 
	{
		flashstatus = INT_FLASH_BUSY;
	}
	else if (FLASH->SR & FLASH_SR_WRPERR)
	{
		flashstatus = INT_FLASH_ERROR_WRP;
	}
	else if (FLASH->SR & (FLASH_SR_OPERR | FLASH_SR_PROGERR | FLASH_SR_PGAERR \
				| FLASH_SR_SIZERR | FLASH_SR_PGSERR | FLASH_SR_MISERR | FLASH_SR_FASTERR | FLASH_SR_RDERR))
	{
		flashstatus = INT_FLASH_ERROR_PROGRAM;
	}
	else
	{
		flashstatus = INT_FLASH_OK;
	}
	return flashstatus;
}

flash_status_e ifl_wait_for_last_operation(void)
{
	__IO flash_status_e status = INT_FLASH_OK;

	// Wait for the FLASH operation to complete by polling on BUSY flag to be reset.
	// Even if the FLASH operation fails, the BUSY flag will be reset and an error flag will be set
	while ((status = fl_getstatus()) == INT_FLASH_BUSY)
	{
	}
	return status;
}


static flash_status_e ifl_erase_page(u32 page)
{
	u32 bank;

	flash_status_e status = INT_FLASH_OK;

	bank = (page >> FLASH_BANK_PAGE_BITS);
	page &= FLASH_BANK_PAGE_MASK;

	// Wait for last operation to be completed
	status = ifl_wait_for_last_operation();
	
	if (status == INT_FLASH_OK)
	{ 
		// if the previous operation is completed, proceed to erase the sector
		if ((page>127) || (bank>1))
		{
			return (INT_FLASH_ERROR_PROGRAM);
		}
		
		__disable_irq();
		if (bank == 1)
			SET_BIT(FLASH->CR, FLASH_CR_BKER);
		else
			CLEAR_BIT(FLASH->CR, FLASH_CR_BKER);

		MODIFY_REG(FLASH->CR, FLASH_CR_PNB, ((page & 0xFFU) << FLASH_CR_PNB_Pos));
		SET_BIT(FLASH->CR, FLASH_CR_PER);
		SET_BIT(FLASH->CR, FLASH_CR_STRT);

		//  Wait for last operation to be completed
		status = ifl_wait_for_last_operation();
		CLEAR_BIT(FLASH->CR, FLASH_CR_PER);
		__enable_irq();
	}
	return status;
}


flash_status_e ifl_program_dword(u32 address, u32 *doubleword)
{
	flash_status_e status = INT_FLASH_OK;

	// Wait for last operation to be completed
	status = ifl_wait_for_last_operation();
	
	if(status == INT_FLASH_OK)
	{
		// if the previous operation is completed, proceed to program the new data
		__disable_irq();

		SET_BIT(FLASH->CR, FLASH_CR_PG);

		// Program first word
		*(u32 *)address = doubleword[0];
		// Barrier to ensure programming is performed in 2 steps, in right order
		__ISB();
		// Program second word
		address += 4;
		*(u32 *)address = doubleword[1];
		__ISB();

		// Wait for last operation to be completed
		status = ifl_wait_for_last_operation();
		
		// if the program operation is completed, disable the PG Bit
		CLEAR_BIT(FLASH->CR, FLASH_CR_PG);
		__enable_irq();
	}
	else
	{
		OS_PRINTF("[ERR1]" NL);
	}
	return (status);
}

static void _clr_err(void)
{
	// Clear pending flags (if any)
	FLASH->SR = FLASH_SR_EOP | FLASH_SR_OPERR | FLASH_SR_PROGERR | FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_SIZERR \
			  | FLASH_SR_PGSERR | FLASH_SR_MISERR | FLASH_SR_FASTERR | FLASH_SR_RDERR | FLASH_SR_OPTVERR | FLASH_SR_BSY;
}

bool int_flash_write(u32 addr, u32 *data, u32 len)
{
	int i;
	int page;
	u32 *p;
	
	if (READ_BIT(FLASH->OPTR, FLASH_OPTR_DBANK) == 0U)
	{
		LOG_ERROR("DBANK=0"); // default is DBANK==1
		return (false);
	}

	if (len % 8)
	{	// support only double-word programming step
		LOG_ERROR("size align");
		return (false);
	}

	ASSERT(((addr) >= FLASH_BASE) && ((addr) < (FLASH_BASE+FLASH_SIZE)), "int_flash_write() ADDR");

	// Unlock the FLASH Program memory
	wd_feed();
	flash_unlock();

	_clr_err();
	p=(u32*)data;

	page = begin_of_page(addr);
	// OS_PRINTF("[wr:%lx, p:%d]" NL, addr, page);

	if (page>=0)
	{
		OS_PRINTF(NL);
		OS_PRINTF("IFL: p %d ", page);
		OS_DELAY(20);
        wd_feed();

		if (ifl_erase_page(page) != INT_FLASH_OK)
		{
			LOG_ERROR("clear error (1)");
			goto err;
		}
		else
		{
			_clr_err();
		}
		if(*(u32*)addr!=0xFFFFFFFFUL)
		{
			LOG_ERROR("clear error (2)");
			goto err;
		}
	}
	else
	{
		OS_PRINTF(".");
	}
	OS_DELAY(10);

	// Data received are double-word multiple
	wd_feed();
	for (i = 0; i<len; i+=8)
	{
		u32 buf[2];	
		buf[0] = *p++;
		buf[1] = *p++;

		if (ifl_program_dword(addr, buf)!= INT_FLASH_OK)
		{
			LOG_ERROR("write error, a=%" PRIX32, addr);
			goto err;
		}
		else
		{
			_clr_err();
		}
		addr += 8;
	}
	flash_lock();
	return (true);
err:
	flash_lock();
	return (false);
}

