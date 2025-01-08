#include "8250_uart.h"
int uart_puts(const char *s)
{
        int len = 0;

        while (*s) {
                uart8250_putc(*s);

                if (*s == '\n')
                        uart8250_putc('\r');
                s++;
                len++;
        }
        return len;
}
