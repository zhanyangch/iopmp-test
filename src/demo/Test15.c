
/*
 * Testcase 15:
 * DMA partially hit error
 */

#include "../utility/riscv_iopmp.h"
#include "platform.h"
#include "plic.h"
#include <stdio.h>

#define printf(arg...) do { char str[0x100]; sprintf(str, ##arg); uart_puts(str); } while (0)

#define USE_NAPOT             0

#define DMA_MD                4

#define ARRAY_SIZE            32

typedef void (*fun_ptr)(void*);

volatile int bus_error_done, iopmp_isr_done, iopmp_read_isr_done, iopmp_fetch_isr_done;
volatile int expect_exception_cause, expect_iopmp_ttype, expect_iopmp_etype;

volatile unsigned char DMA_RO_Array[ARRAY_SIZE] __attribute__ ((aligned(ARRAY_SIZE)));
volatile unsigned char DMA_WO_Array[ARRAY_SIZE] __attribute__ ((aligned(ARRAY_SIZE)));

volatile unsigned char *TEST_RW_Ptr;
int no_w;
int fetch_flag;
int add_perm_done;
int iopmp_k;

RISCV_IOPMP_OPS *iopmp_ops;
extern RISCV_IOPMP_OPS riscv_iopmp_ops;
int CPU_MD[] = {0, 1, 2};

#define DMA_STATUS_COMPLETE 1
#define DMA_STATUS_ERROR    2

void iopmp_irq_init() {
	plic_enable_interrupt(IRQ_IOPMP_SOURCE, 1);
}

long except_handler(long cause, long epc)
{
	unsigned short inst_base = *(unsigned short *)epc;
	bus_error_done++;

	if (cause != expect_exception_cause) {
		printf("Failure: Get an unexpected exception!\n");
		while (1);
	}

	if (cause == TRAP_M_L_ACC_FAULT) {
		printf("[Load Access Fault]\n");
		/* Update return address */
		if ((inst_base & 0x3) != 0x3) {
			/* 16-bit instruction */
			epc += 2;
		} else if (((inst_base >> 2) & 0x7) != 0x7) {
			/* 32-bit instruction */
			epc += 4;
		} else {
			/* 64-bit instruction */
			epc += 8;
		}
	}

	if (cause == TRAP_M_S_ACC_FAULT) {
		printf("[Store Access Fault]\n");
		/* Update return address */
		if ((inst_base & 0x3) != 0x3) {
			/* 16-bit instruction */
			epc += 2;
		} else if (((inst_base >> 2) & 0x7) != 0x7) {
			/* 32-bit instruction */
			epc += 4;
		} else {
			/* 64-bit instruction */
			epc += 8;
		}
	}

	if (cause == TRAP_M_I_ACC_FAULT) {
		printf("[Instruction Access Fault]\n");
		while (1); //TODO: jump to other function which have excute permision
	}
	expect_exception_cause = 0;
	return epc;
}

volatile int error_ttype, error_etype;
void iopmp_irq_handler(void)
{
	if (iopmp_ops->get_irq_pending(DEV_IOPMP0)) {
		iopmp_isr_done++;
		error_ttype = iopmp_ops->get_error_ttype(DEV_IOPMP0);
		error_etype = iopmp_ops->get_error_etype(DEV_IOPMP0);
		switch (error_ttype) {
			case TTYPE_READ:
				printf("[IOPMP Read Error]\n");
				break;
			case TTYPE_WRITE:
				printf("[IOPMP Write Error]\n");
				break;
			case TTYPE_FETCH:
				printf("[IOPMP Fetch Error]\n");
				break;
			default:
				printf("[IOPMP Reserved Error]\n");
				break;
		}
		switch (error_etype) {
			case ETYPE_READ:
				printf("[Read Permssion Error]\n");
				break;
			case ETYPE_WRITE:
				printf("[Write Permssion Error]\n");
				break;
			case ETYPE_FETCH:
				printf("[Fetch Permssion Error]\n");
				break;
			case ETYPE_PARTIAL_HIT:
				printf("[Partial Hit Error]\n");
				break;
			case ETYPE_NO_HIT:
				printf("[No Hit Error]\n");
				break;
			case ETYPE_RRID:
				printf("[RRID Error]\n");
				break;
			default:
				printf("[IOPMP Reserved Error]\n");
				break;
		}
		iopmp_ops->clear_irq_pending(DEV_IOPMP0);
	}
}

#define ENTRY_NUM_PER_MD 6
int srcmd_fmt, mdcfg_fmt;
int main(void)
{
	uart_init();
	int md_num;
	iopmp_ops = &riscv_iopmp_ops;
	int dma_stat;
	printf("\nQEMU IOPMP testcase15\n");

	srcmd_fmt = iopmp_ops->get_srcmd_fmt(DEV_IOPMP0);
	mdcfg_fmt = iopmp_ops->get_mdcfg_fmt(DEV_IOPMP0);

	if (srcmd_fmt != 0 || mdcfg_fmt != 0) {
		printf("Failure: This case only supports srcmd_fmt0 mdcfg_fmt0\n");
		while (1);
	}

	/* Prepare the data */
	*(volatile unsigned int *)&DMA_RO_Array[0] = 0x5555AAAA;
	*(volatile unsigned int *)&DMA_WO_Array[0] = 0x7777CCCC;

	md_num = iopmp_ops->get_md_num(DEV_IOPMP0);
	iopmp_k = iopmp_ops->get_md_entry_num(DEV_IOPMP0) + 1;
	printf("IOPMP configuration\n");
	printf("hwcfg0 0x%x\n", iopmp_ops->get_hwcfg(DEV_IOPMP0, 0));
	printf("mdcfg_fmt: %d\n", mdcfg_fmt);
	printf("srcmd_fmt: %d\n", srcmd_fmt);
	printf("number of MD: %d\n", md_num);
	printf("number of RRID: %d\n", iopmp_ops->get_rrid_num(DEV_IOPMP0));
	printf("number of entries: %d\n",  iopmp_ops->get_entry_num(DEV_IOPMP0));
	printf("number of prior entry: %d\n", iopmp_ops->get_prior_entry_num(DEV_IOPMP0));
	if (mdcfg_fmt != 0) {
		printf("k value: %d\n", iopmp_k);
	}

	if (mdcfg_fmt == 0) {
		iopmp_k = ENTRY_NUM_PER_MD;
		for(int i=0; i < md_num; i++){
			iopmp_ops->set_mdcfg(DEV_IOPMP0, i, iopmp_k * (i + 1));
		}
	}

	/* Setup IOPMP */
	if (iopmp_k < 3) {
		printf("Failure: The number of IOPMP entry is insufficient for the NAPOT mode demonstration.\n");
		while (1);
	}
	printf("Configure IOPMP entries with NAPOT scheme.\n");

	/* Setup MDs for DEV_IOPMP0 */
	iopmp_ops->srcmd_add_md(DEV_IOPMP0, CPU_RRID, CPU_MD[0]);
	/* 0x0 ~ 0xFFFFFFFF, executable, writable, readable */
	iopmp_ops->napot_config(DEV_IOPMP0, CPU_MD[0] * iopmp_k, (void *)0, 0xFFFFFFFF, ENTRY_CFG_XWR(ENTRY_X_ON, ENTRY_W_ON, ENTRY_R_ON));

        iopmp_ops->srcmd_add_md(DEV_IOPMP0, DMA_RRID, DMA_MD);
	/* config half array size permission */
	iopmp_ops->napot_config(DEV_IOPMP0, DMA_MD * iopmp_k, (void *)DMA_RO_Array, ARRAY_SIZE/2, ENTRY_CFG_XWR(ENTRY_X_OFF, ENTRY_W_OFF, ENTRY_R_ON));
	iopmp_ops->napot_config(DEV_IOPMP0, DMA_MD * iopmp_k + 1, (void *)DMA_WO_Array, ARRAY_SIZE/2, ENTRY_CFG_XWR(ENTRY_X_OFF, ENTRY_W_ON, ENTRY_R_OFF));

	int error_reaction_ctrl = (ERR_CFG_CTRL_IE_ENABLE | ERR_CFG_CTRL_RS_DISABLE);
	iopmp_ops->error_reaction(DEV_IOPMP0, error_reaction_ctrl);

	iopmp_irq_init();
	iopmp_ops->enable(DEV_IOPMP0);

        /* Enable the Machine External interrupt */
	set_csr(CSR_MIE, MIP_MEIP);
	/* Enable interrupts in general. */
	set_csr(CSR_MSTATUS, MSTATUS_MIE);

	printf("DMA memory copy (partially hit error)\n");
	dma_mem_copy((long)DMA_RO_Array, (long)DMA_WO_Array, ARRAY_SIZE);

	while (dma_get_status() == 0);
	dma_stat = dma_get_status();
	if (dma_stat != DMA_STATUS_ERROR) {
		printf("Failure: DMA status is not ERROR\n");
		while (1);
	}

	if (*(volatile unsigned int *)&DMA_WO_Array != 0x7777CCCC) {
		printf("Failure: WO Array Data is wrong\n");
		while (1);
	}

	while (iopmp_isr_done == 0);
	if (error_etype != ETYPE_PARTIAL_HIT) {
		printf("Failure: etype is wrong\n");
	}

	printf("IOPMP testcase15 success.\n");
	while(1);
}
