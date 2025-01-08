#include <stdio.h>
#include "platform.h"
#include "csr.h"
#include "plic.h"

void default_irq_handler(void)
{
	while(1);
}

void iopmp_irq_handler(void)  __attribute__((weak, alias("default_irq_handler")));

void external_interrupt_handler(unsigned int irq_source)
{
	switch (irq_source)
	{
	case IRQ_IOPMP_SOURCE:
		iopmp_irq_handler();
		break;
	default:
		default_irq_handler();
		break;
	}

	/* Disable interrupt in general to restore context */
	clear_csr(CSR_MSTATUS, MSTATUS_MIE);
}