#ifndef __VIRT_H__
#define __VIRT_H__
#include "../../utility/riscv_iopmp.h"
#include "dma.h"
#define INIT_STACK              0x84000000

#define IRQ_UART0_SOURCE        10
#define IRQ_IOPMP_SOURCE        12
#define IRQ_IOPMPDMA_SOURCE     13

#define PLIC_BASE               (0xc000000UL)
#define UART0_BASE              (0x10000000)
#define	IOPMP0_BASE             (0x10200000)
#define	IOPMP1_BASE             (0x10300000)
#define IOPMPDMA_BASE           (0x10400000)

#define	VIRT_IOPMP0            ((IOPMP_RegDef *) IOPMP0_BASE)
#define	VIRT_IOPMP1            ((IOPMP_RegDef *) IOPMP1_BASE)
#define VIRT_IOPMPDMA          ((IOPMPDMA_RegDef *) IOPMPDMA_BASE)

#endif
