#include "os.h"
#include "hardware.h"

#include "spi.h"
#include "flash_lib.h"

#define     CMD_SR_READ         0x05    // read status register
#define     CMD_READ_DATA       0x03    // 
#define     CMD_ERASE_4K        0x20    // 
#define     CMD_ERASE_64K       0xD8    // 
#define     CMD_CHIP_ERASE      0xC7    // 
#define     CMD_PAGE_PROG       0x02    // 
#define     CMD_BYTE_PROG       0x02    // 
#define     CMD_PWRDN           0xB9    // power-down
#define     CMD_FAST_READ       0x0B
#define     CMD_AAI_PROG        0xAD    // auto address increment programming
#define     CMD_WRSR            0x01    // write status
#define     CMD_WREN            0x06    // write enable
#define     CMD_WRDI            0x04    // write disable
#define     CMD_RDID            0x90    // Manufacturer device ID
#define     CMD_JEDEC_ID        0x9F    // 

#define     SR_BUSY         (1<<0)
#define     SR_WEL          (1<<1)  // write enable latch
#define     SR_PROTECT      (1<<7)

#define     CMD_SECTOR_ERASE    CMD_ERASE_4K  // Sector Erase
#define     SECTOR_SIZE         4096
#define     SECTOR_SIZE_MASK    (SECTOR_SIZE-1) // 0x0FFF

#define     CMD_BLOCK_ERASE     CMD_ERASE_64K // Block Erase 
#define     BLOCK_SIZE          65536           // 0x10000
#define     BLOCK_SIZE_MASK     (BLOCK_SIZE-1)  // 0x0FFFF

#define     MAX_WRITE_SIZE          256
#define     MAX_WRITE_SIZE_MASK     (MAX_WRITE_SIZE-1)

#define flash_spi_init()        HW_FLASH_SPI_INIT()
#define flash_spi_wr(x)         HW_FLASH_SPI_TRANSFER(x)
#define flash_spi_flush_buffer() 

#define FLASH_SPI   (5) // unused ID

#define FLASH_CACHED_AREA (1024*64)

enum {
    FLASH_TYPE_UNKNOWN=0,
    FLASH_TYPE_SST25V,
    FLASH_TYPE_W25X,
    FLASH_TYPE_ERROR, // not supported type
};

typedef struct {
    const u32 device_id; // 
    const u8  local_id;  // 
    const u8  size;      // size in MiB
    const ascii *name;   // 
} device_id_t;

const device_id_t FLASH_DEVICE_ID[]  = {
    {0xEF3016, FLASH_TYPE_W25X,    4, "W25X"},      // Winbond
    {0xEF4016, FLASH_TYPE_W25X,    4, "W25Q32"},    // Winbond
    {0xEF4018, FLASH_TYPE_W25X,   16, "W25Q128JV"}, // Winbond
    {0xEF6018, FLASH_TYPE_W25X,   16, "W25Q128JW"}, // Winbond
    {0,        FLASH_TYPE_UNKNOWN, 4, "UNKNOWN"},   // table end
};

#define CACHE_SECTORS_COUNT          2 // number of cache sectors
#define CACHE_FLUSH_TIME             (30*OS_TIMER_SECOND)

typedef struct {
    u8   buf[SECTOR_SIZE];
    s32  addr;              // sector address
    os_timer_t  write_time; //
    bool flush_needed;      //
} flash_cache_sector_t;

flash_cache_sector_t flash_cache[CACHE_SECTORS_COUNT]; 
volatile u8 flash_cache_last_used_index;

static bool flash_changed = false;
static u8 flash_size = 0;

bool flash_init (void);
u8 flash_read_status (void);

bool flash_data_erase (u32 addr, u32 len);
bool flash_read_data (u8 *dest, u32 addr, u32 len);
bool flash_write_data (u32 addr, u8 *src, u32 len);
bool flash_write_data_noerase (u32 addr, u8 *src, u16 len);

void little_dly (void);
void flash_wr_addr (u32 addr);
void flash_wr_u32 (u32 data);
void flash_busy_wait (void);
bool flash_write_sector (u32 addr, u8 *src); 
bool flash_erase_sector (u32 addr);
void flash_write_enable (void);
flash_cache_sector_t   *flash_read_sector (u32 addr);
flash_cache_sector_t   *flash_get_suited_cache_sector (void); 
void flash_cache_flush_sector(flash_cache_sector_t *flash_cache_sector);
void flash_flush_cache (void);

volatile u8 flash_type=FLASH_TYPE_UNKNOWN;

void little_dly (void)
{
    int i;
    for (i=0;i<50;i++)
    {
    }
}

void flash_write_enable (void)
{
    HW_FLASH_CS_LOW;
    flash_spi_wr(CMD_WREN); // 
    HW_FLASH_CS_HI;
    little_dly (); 
}

bool flash_init (void)
{
    u32 id=0;
    u8  i;
    
    for(i=0; i<=1; i++)
    {
        flash_cache[i].addr = -1; // empty block
        flash_cache[i].write_time = 0;
        flash_cache[i].flush_needed = false;
    }
    flash_cache_last_used_index = 0;

    HW_FLASH_CS_HI;
    HW_FLASH_CS_INIT;
    flash_spi_init();
    HW_FLASH_CS_HI;

    flash_spi_flush_buffer();
    OS_DELAY(10);

    flash_busy_wait();

    HW_FLASH_CS_LOW;
    flash_spi_wr(CMD_JEDEC_ID); // 
    id=flash_spi_wr(0);  // manufacturer
    id<<=8;
    id+=flash_spi_wr(0); // memory type
    id<<=8;
    id+=flash_spi_wr(0); // capacity
    HW_FLASH_CS_HI;

    if ((id==0) || (id==0xFFFFFF))
    {   // flash does not work or does not support JEDEC_ID
        OS_PRINTF("[ID=0x%06" PRIX32 "]", id);
        return (false);
    }
    for (i=0; i<100; i++)
    {
        if (FLASH_DEVICE_ID[i].device_id == id)
        {
            OS_PRINTF ("[%s] ", FLASH_DEVICE_ID[i].name);
            if (FLASH_DEVICE_ID[i].local_id == FLASH_TYPE_ERROR)
            {
                OS_PRINTF("[ID=0x%06" PRIX32 "]", id);
                return (false); // known, but unsupported memory
            }

            flash_type=FLASH_DEVICE_ID[i].local_id; 
            break;
        }
        if (FLASH_DEVICE_ID[i].local_id == FLASH_TYPE_UNKNOWN)
        {   // end of lid, unknown flash !
            // try to use it as most used standard (like Winbond)
            flash_type=FLASH_TYPE_W25X;
            OS_PRINTF ("[UNKNOWN DEVICE: 0x%06" PRIX32 "] ", id);
            break; 
        }
    }
    flash_size = FLASH_DEVICE_ID[i].size;

    flash_write_enable ();

    HW_FLASH_CS_LOW;
    flash_spi_wr(CMD_WRSR);
    flash_spi_wr(0);        // enable write to all sectors
    HW_FLASH_CS_HI;

    little_dly();
    flash_busy_wait();

    return (true);

}

u32 flash_get_size(void)
{
    OS_ASSERT(flash_size>=4, "flash size error");
    return (flash_size * 1024UL * 1024UL);
}

u8 flash_read_status (void)
{
    u8   retval;
    
    flash_spi_flush_buffer();

    HW_FLASH_CS_LOW;

    flash_spi_wr(CMD_SR_READ); 
    retval = flash_spi_wr(0x00); 
    HW_FLASH_CS_HI;

    return (retval);
}

void flash_wr_u32 (u32 data)
{
    flash_spi_wr((data)     & 0xFF); 
    flash_spi_wr((data>>8)  & 0xFF); 
    flash_spi_wr((data>>16) & 0xFF); 
    flash_spi_wr((data>>24) & 0xFF);
}


void flash_wr_addr (u32 addr)
{
    flash_spi_wr((addr>>16) & 0xFF); 
    flash_spi_wr((addr>>8) & 0xFF); 
    flash_spi_wr((addr) & 0xFF);
}

void flash_busy_wait (void)
{
    little_dly();
    while (flash_read_status() & SR_BUSY) 
        ;
}

bool flash_write_data_noerase (u32 addr, u8 *src, u16 len)
{   // zapis bez kontroly,predpokladame vycistenou oblast
    // a zapise vzdy primo, nepouzije cache
    u8 i;
    u16 wr_len;

    while (len)
    {       
        if (len>MAX_WRITE_SIZE)
        {
            wr_len=MAX_WRITE_SIZE;
        }
        else
        {
            wr_len=len;
        }
        if (wr_len + (addr & MAX_WRITE_SIZE_MASK) > MAX_WRITE_SIZE)
        {   // dont write over page border
            wr_len = MAX_WRITE_SIZE-(addr & MAX_WRITE_SIZE_MASK);
        }
        len-=wr_len;

        for(i=0; i<CACHE_SECTORS_COUNT; i++)
        {
            if ((flash_cache[i].addr & ~SECTOR_SIZE_MASK) == (addr & ~SECTOR_SIZE_MASK))
            {   // when writing to cached sector update also cache
                if (wr_len + (addr & SECTOR_SIZE_MASK) > SECTOR_SIZE)
                    memcpy (flash_cache[i].buf + (addr & SECTOR_SIZE_MASK), src, SECTOR_SIZE-(addr & SECTOR_SIZE_MASK));
                else
                    memcpy (flash_cache[i].buf + (addr & SECTOR_SIZE_MASK), src, wr_len);

                flash_cache[i].write_time = os_timer_get(); 
                flash_cache_last_used_index = i;
                break;
            }
        }

        flash_busy_wait();
        flash_write_enable();

        if (flash_type==FLASH_TYPE_SST25V)
        {
            if  (wr_len<2)
                return (false); 

            HW_FLASH_CS_LOW;
            flash_spi_wr(CMD_AAI_PROG); 
            flash_wr_addr (addr);
            addr+=wr_len;
            flash_spi_wr(*(src++));
            flash_spi_wr(*(src++));
            HW_FLASH_CS_HI;
            wr_len-=2;

            while (wr_len>1)
            {
                little_dly();
                // flash_spi_flush_buffer ();
                // flash_busy_wait();
                HW_FLASH_CS_LOW;
                flash_spi_wr(CMD_AAI_PROG); 
                flash_spi_wr(*(src++));
                flash_spi_wr(*(src++));
                HW_FLASH_CS_HI;
                wr_len-=2;
            }
            little_dly();
            HW_FLASH_CS_LOW;
            flash_spi_wr(CMD_WRDI); 
            HW_FLASH_CS_HI;
            little_dly();
        }
        else if (flash_type==FLASH_TYPE_W25X)
        {
            HW_FLASH_CS_LOW;
            flash_spi_wr(CMD_PAGE_PROG);    
            flash_wr_addr (addr);
            addr+=wr_len;

            while (wr_len--)
            {
                flash_spi_wr(*(src++));
            }
            HW_FLASH_CS_HI;
            little_dly();
        }
    }
    return (true);
}

bool flash_write_sector (u32 addr, u8 *src)
{   // we assume here the sector is erased
    addr&=(~SECTOR_SIZE_MASK);

    return (flash_write_data_noerase (addr, src, SECTOR_SIZE));
     
}

bool flash_erase_sector (u32 addr)
{
    addr&=(~SECTOR_SIZE_MASK);

    flash_spi_flush_buffer();
    flash_busy_wait();

    flash_write_enable();

    HW_FLASH_CS_LOW;
    flash_spi_wr(CMD_SECTOR_ERASE);
    flash_wr_addr (addr);
    HW_FLASH_CS_HI;
    return (true);
}



bool flash_data_erase (u32 addr, u32 len)
{
    u8 i;   

    flash_spi_flush_buffer();
    flash_busy_wait();

    while (len)
    {
        if (((addr&BLOCK_SIZE_MASK) == 0) &&
             (len>=BLOCK_SIZE))
        {   // erase whole block
            flash_write_enable ();
            HW_FLASH_CS_LOW;
            flash_spi_wr(CMD_BLOCK_ERASE);
            flash_wr_addr (addr);
            HW_FLASH_CS_HI;

            for(i=0; i<CACHE_SECTORS_COUNT; i++)
            {
                if ((addr & ~BLOCK_SIZE_MASK) == (flash_cache[i].addr & ~BLOCK_SIZE_MASK))
                {
                    flash_cache[i].addr=-1;
                    flash_cache[i].flush_needed=false;
                }
            }

            len-=BLOCK_SIZE;
            addr+=BLOCK_SIZE;           
        }
        else if (((addr&SECTOR_SIZE_MASK) == 0) &&
             (len>=SECTOR_SIZE))
        {   // erase whole page 
            flash_erase_sector (addr);

            for(i=0; i<CACHE_SECTORS_COUNT; i++)
            {
                if ((addr & ~SECTOR_SIZE_MASK) == flash_cache[i].addr)
                {
                    flash_cache[i].addr=-1;
                    flash_cache[i].flush_needed=false;
                }
            }

            len-=SECTOR_SIZE;
            addr+=SECTOR_SIZE;
        }
        else 
        {   // the last option is read->change->write
            flash_cache_sector_t *flash_cache_sector;
            u16 wr_len;
            u16 i;

            wr_len = SECTOR_SIZE - (addr & SECTOR_SIZE_MASK);
            if (wr_len>len)
                wr_len=len;

            flash_cache_sector = flash_read_sector (addr);
            for (i=0; i<wr_len; i++)
            {
                if (flash_cache_sector->buf[(addr&SECTOR_SIZE_MASK)+i] == 0xFF)
                    continue; // flash empty
                
                // need to write
                memset (&flash_cache_sector->buf[addr&SECTOR_SIZE_MASK], 0xFF, wr_len); 
                flash_erase_sector (addr); 
                flash_write_sector (addr&(~SECTOR_SIZE_MASK), flash_cache_sector->buf);
                break;
            }

            len -= wr_len;
            addr = (addr & (~SECTOR_SIZE_MASK)) + SECTOR_SIZE;
        }
        flash_busy_wait ();
    }

    return (true);
}


bool flash_read_data (u8 *dest, u32 addr, u32 len)
{
    flash_cache_sector_t *flash_cache_sector;
    u8 i;

    // first search in cache
    for(i=0; i<CACHE_SECTORS_COUNT; i++)
    {
        if ( ((flash_cache[i].addr & ~SECTOR_SIZE_MASK) == (addr & ~SECTOR_SIZE_MASK))
            && ((addr & SECTOR_SIZE_MASK)+len <= SECTOR_SIZE) )
        {
            memcpy (dest, flash_cache[i].buf + (addr & SECTOR_SIZE_MASK), len);
            flash_cache_last_used_index = i;
            return (true);
        }
    }

    if ( (addr < FLASH_CACHED_AREA) && // is it inside cached space
         ((addr & SECTOR_SIZE_MASK)+len <= SECTOR_SIZE) ) // not over sector border
    {
        flash_cache_sector = flash_read_sector (addr & ~SECTOR_SIZE_MASK);
        memcpy (dest, flash_cache_sector->buf + (addr & SECTOR_SIZE_MASK), len);
    }
    else
    {
        // plain read without using cache
        flash_spi_flush_buffer();
        flash_busy_wait();

        HW_FLASH_CS_LOW;
        flash_spi_wr(CMD_READ_DATA); 
        flash_wr_addr (addr);
        while (len--)
        {
            *(dest++) = flash_spi_wr(0x00); 
        }
        HW_FLASH_CS_HI;
    }
    return (true);
}

flash_cache_sector_t   *flash_get_suited_cache_sector (void)
{   // return pointer to suitable cache for next operation 
    // and update  information about cached-sektor

#if (CACHE_SECTORS_COUNT != 2)
 #error "Current algorithm does not work for more caching, need to refactor"
#endif

    if (flash_cache[0].flush_needed == flash_cache[1].flush_needed)
    {   
        if (flash_cache_last_used_index == 0)
        {
            flash_cache_last_used_index = 1;
            return (&flash_cache[1]);
        }
    }
    else
    {
        if (flash_cache[0].flush_needed == true)
        {
            flash_cache_last_used_index = 1;
            return (&flash_cache[1]);
        }   
    }

    flash_cache_last_used_index = 0;
    return (&flash_cache[0]); 
}

flash_cache_sector_t   *flash_read_sector (u32 addr)
{
    flash_cache_sector_t *flash_cache_sector;
    u16 i;
    addr &= ~SECTOR_SIZE_MASK;

    
    for(i=0; i<CACHE_SECTORS_COUNT; i++)
    {
        if (addr == flash_cache[i].addr)
        {
            flash_cache_last_used_index = i;
            return (&flash_cache[i]);
        }
    }
    
    flash_cache_sector = flash_get_suited_cache_sector();

    flash_cache_flush_sector(flash_cache_sector);

    flash_spi_flush_buffer();
    flash_busy_wait();

    HW_FLASH_CS_LOW;
    flash_spi_wr(CMD_READ_DATA); 
    flash_wr_addr (addr);
    for (i=0; i<SECTOR_SIZE; i++)
    {
        flash_cache_sector->buf[i] = flash_spi_wr(0x00); 
    }
    HW_FLASH_CS_HI;
    flash_cache_sector->addr = addr;
    return (flash_cache_sector);
}

bool flash_write_data (u32 addr, u8 *src, u32 len)
{
    flash_cache_sector_t *flash_cache_sector;
    u16 i;
    u16 wr_len;
    bool need_erase;
    bool need_write;

    flash_spi_flush_buffer();
    flash_busy_wait();
    flash_changed=false;
    
    while (len)
    {
        need_erase = false;
        need_write = false;

        wr_len = SECTOR_SIZE - (addr & SECTOR_SIZE_MASK);
        if (wr_len>len)
            wr_len=len;

        flash_cache_sector = flash_read_sector (addr);
        for (i=0; i<wr_len; i++)
        {
            if (flash_cache_sector->buf[i+(addr&SECTOR_SIZE_MASK)] != src[i])
            {   // data differs, we will need to write
                need_write=true;
                flash_changed=true;
                break;
            }
        }
        if (need_write)
        {
            for (i=0; i<wr_len; i++)
            {
                if (flash_cache_sector->buf[i+(addr&SECTOR_SIZE_MASK)] != 0xFF)
                {   // not empty, need to erase
                    need_erase=true;
                }
                flash_cache_sector->buf[i+(addr&SECTOR_SIZE_MASK)] = src[i];
            }
            if (need_erase)
            {   // overwrite sector
                flash_cache_sector->flush_needed = true;
                flash_cache_sector->write_time = os_timer_get();
                // write it later using flash_cache_flush_sector()
            }
            else
            {
                flash_write_data_noerase (addr, src, wr_len);           
            }
        }
        
        src += wr_len;
        len -= wr_len;
        addr = (addr & (~SECTOR_SIZE_MASK)) + SECTOR_SIZE;
        // when running over page border, continue 
    }

    return (true);
}

bool flash_content_changed(void)
{   // last write operation change detection
    return (flash_changed);
}

void flash_cache_flush_sector(flash_cache_sector_t *flash_cache_sector)
{
    if (flash_cache_sector == NULL)
        return;

    if (flash_cache_sector->flush_needed == true)
    {
        flash_erase_sector (flash_cache_sector->addr);
        flash_write_sector (flash_cache_sector->addr, flash_cache_sector->buf);
        flash_cache_sector->write_time = 0;
        flash_cache_sector->flush_needed = false;
    }
}

void flash_flush_cache(void)
{
    u8 i;
    
    for(i=0; i<CACHE_SECTORS_COUNT; i++)
    {
        flash_cache_flush_sector(&flash_cache[i]);
    }
}

void flash_maintenace_task(void)
{
    u8 i;
    
    // after timeout flush cache 
    for(i=0; i<CACHE_SECTORS_COUNT; i++)
    {
        if (flash_cache[i].write_time == 0)
            continue;

        if ((os_timer_t)(os_timer_get()-flash_cache[i].write_time) < CACHE_FLUSH_TIME)
            continue;

        flash_cache_flush_sector(&flash_cache[i]);
        flash_cache[i].write_time = 0;
    }
}

