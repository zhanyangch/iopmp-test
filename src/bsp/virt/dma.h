#ifndef __DMA_H__
#define __DMA_H__
typedef struct
{
    volatile unsigned int SRC;                  /* 0x00 */
    volatile unsigned int DST;                  /* 0x04 */
    volatile unsigned int SZ;                   /* 0x08 */
    volatile unsigned int START;                /* 0x0C */
    volatile unsigned int STATUS;               /* 0x10 */
} IOPMPDMA_RegDef;

void dma_mem_copy(int src, int dest, int size);
int dma_get_status();
void dma_clear_status(int status);
#endif