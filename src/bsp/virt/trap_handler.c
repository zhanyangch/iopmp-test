#include <stdio.h>
#include "platform.h"
#include "csr.h"

#define printf(arg...) do { char str[0x100]; sprintf(str, ##arg); uart_puts(str); } while (0)

extern void external_interrupt_handler(unsigned int irq_source);
extern void bwei_handler();

__attribute__((weak)) long except_handler(long cause, long epc)
{
	/* Unhandled Exception */
	printf("Unhandled Exception : mcause = 0x%x, mepc = 0x%x\n", (unsigned int)cause, (unsigned int)epc);

	return epc;
}

__attribute__((weak,used)) void interrupt_entry(long cause)
{
	long mepc = read_csr(CSR_MEPC);
	long mstatus = read_csr(CSR_MSTATUS);

	if ((cause & MCAUSE_CAUSE) == 11) {
		external_interrupt_handler(plic_claim());
	}
	/* Restore CSR */
	write_csr(CSR_MSTATUS, mstatus);
	write_csr(CSR_MEPC, mepc);
}

__attribute__((weak,used)) void except_entry(long cause)
{
	long epc = read_csr(CSR_MEPC);
	epc = except_handler(cause, epc);
	write_csr(CSR_MEPC, epc);
}
