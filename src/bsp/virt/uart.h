#ifndef __UART_H__
#define __UART_H__
#include "8250_uart.h"
#include "platform.h"
static inline void uart_init() {
        uart8250_init(UART0_BASE, 1843200, 115200, 0, 1);
}
#endif