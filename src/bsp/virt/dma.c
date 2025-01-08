#include "dma.h"
#include "platform.h"

void dma_mem_copy(int src, int dest, int size) {
	DEV_DMA->SRC = src;
	DEV_DMA->DST = dest;
	DEV_DMA->SZ = size;
	DEV_DMA->START = 1;
}

int dma_get_status() {
	return DEV_DMA->STATUS;
}

void dma_clear_status(int status) {
	DEV_DMA->STATUS = status;
}