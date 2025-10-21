#include "common.h"
#include "tty.h"

#define TTY_USB  0
#define TTY_NET  1
#define TTY_BLE  1

#if TTY_USB
  #include "usb_device.h"
  #include "usbd_cdc_if.h"
#endif // ! TTY_USB
       //
#if TTY_NET
__WEAK void server_send(const void *buf, size_t count)
{
}
#endif // ! TTY_NET

#if TTY_BLE
__WEAK void ble_send(const void *buf, size_t count)
{
}
#endif // ! TTY_BLE

#ifndef TTY_ON_UART
    #warning "No TTY uart defined"
#endif // ! TTY_ON_UART


static tty_parse_callback_t _rx_callback = NULL;

#if TTY_USB
static char _usb_tty_buf[TTY_BUF_SIZE];
static int  _usb_rx_len = 0;
#endif // ! TTY_USB

int _write (int fd, const void *buf, size_t count)
{   // gcc stdout
    char *ptr = (char *)buf;
    size_t n = count;

#if TTY_USB
    if (usb_device_connected())
    {
        int limit = 10;

        while (usbd_cdc_tx((uint8_t *)buf, count) == USBD_BUSY)
        {
            if (--limit == 0)
                break;
            OS_DELAY(1);
        }
    }
#endif // ! TTY_USB
#if TTY_NET
    server_send(buf, count);
#endif // ! TTY_NET
#if TTY_BLE
    ble_send(buf, count);
#endif // ! TTY_BLE
    while (n--)
    {
        if (! TTY_UART_PUTCHAR(*ptr++))
            break;
    }
    return (count-n);
}

int _read (int fd, const void *buf, size_t count)
{   // gcc stdin
    return (0);
}

#if TTY_USB
static void _usb_rx_handler(uint8_t *buf, uint32_t len)
{
    static char rx_buf[TTY_BUF_SIZE];
    static int rx_len = 0;
    u32 i;
    char ch;

    for (i=0; i<len; i++)
    {
        ch = buf[i];

        if ((ch == '\r') || (ch == '\n'))
        {
            ch = '\0';
        }
        rx_buf[rx_len] = ch;

        if (ch =='\0')
        {
            if (rx_len>0)
            {
                if (_usb_rx_len)
                {   // previous data not processed yet
                    OS_ERROR("USB RX overflow !");
                }
                else
                {   // dont process data here, it is called from ISR
                    // copy for asynchronous processing
                    memcpy(_usb_tty_buf, rx_buf, rx_len+1);
                    _usb_rx_len = rx_len;
                }
                rx_len = 0;
            }
        }
        else if (rx_len < (TTY_BUF_SIZE-1))
            rx_len++;
    }
}
#endif // ! TTY_USB

void tty_rx_task(void)
{
    static char uart_rx_buf[TTY_BUF_SIZE];
    static int  uart_rx_len = 0;

    int ch;

#if TTY_USB
    // process USB RX data
    if (_usb_rx_len)
    {
        if (_rx_callback != NULL)
            _rx_callback(_usb_tty_buf);
        _usb_rx_len = 0;
    }
#endif // ! TTY_USB

    // process UART RX data
    while ((ch = TTY_UART_GETCHAR()) >= 0)
    {
        if ((ch == '\r') || (ch == '\n'))
        {
            ch = '\0';
        }
        uart_rx_buf[uart_rx_len] = ch;

        if (ch =='\0')
        {
            if (uart_rx_len>0)
            {
                if (_rx_callback != NULL)
                    _rx_callback(uart_rx_buf);

                uart_rx_len = 0;
            }
        }
        else if (uart_rx_len < (TTY_BUF_SIZE-1))
            uart_rx_len++;
    }
}

void tty_put_text(const char *text)
{
    while (*text != '\0')
    {
        TTY_UART_PUTCHAR(*text);
        text++;
    }
}

bool tty_init(tty_parse_callback_t callback)
{
    TTY_UART_INIT(115200);
    // TTY_UART_PUTCHAR(0x00);
    // TTY_UART_PUTCHAR(0xFF);
    _rx_callback = callback;
#if TTY_USB
    usbd_cdc_rx_init(_usb_rx_handler);
#endif // ! TTY_USB
    return (true);
}

