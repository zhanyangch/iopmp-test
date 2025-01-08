#ifndef __PLATFORM_H__
#define __PLATFORM_H__
#include "virt.h"
#include "riscv.h"
#include "csr.h"
#include "printf.h"
#include "plic.h"
#include "uart.h"

#define VIRT_PLIC_BASE            PLIC_BASE
#define DEV_IOPMP0                VIRT_IOPMP0
#define DEV_IOPMP1                VIRT_IOPMP1
#define DEV_DMA                   VIRT_IOPMPDMA

// For testcase
#define IOPMP0_REGION_START                0x0
#define IOPMP1_PARALLEL_REGION_START       0x84000000

#define RAM_BASE              0x80000000
#define RAM_SIZE              0x08000000 //128Mib
#define STACK_BASE            0x84000000
#define STACK_SIZE            0x01000000
#endif
