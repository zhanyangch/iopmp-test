
/*
 * Testcase 14:
 * Stall DMA transcation (Stall fault disable) for IOPMP1 region (0x84000000 ~ 0xFFFFFFFF)
 */

#include "../utility/riscv_iopmp.h"
#include "platform.h"
#include "plic.h"
#include <stdio.h>

#define printf(arg...) do { char str[0x100]; sprintf(str, ##arg); uart_puts(str); } while (0)

#define USE_NAPOT             0

#define ARRAY_SIZE            64

typedef void (*fun_ptr)(void*);

volatile int bus_error_done, iopmp_isr_done, iopmp_read_isr_done, iopmp_fetch_isr_done;
volatile int expect_exception_cause, expect_iopmp_ttype, expect_iopmp_etype;

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

void iopmp_irq_handler(void)
{
	int error_ttype, error_etype;
	if (iopmp_ops->get_irq_pending(DEV_IOPMP1)) {
		iopmp_isr_done++;
		error_ttype = iopmp_ops->get_error_ttype(DEV_IOPMP1);
		error_etype = iopmp_ops->get_error_etype(DEV_IOPMP1);
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
		iopmp_ops->clear_irq_pending(DEV_IOPMP1);
	}
}

#define ENTRY_NUM_PER_MD 6
int srcmd_fmt, mdcfg_fmt;
int main(void)
{
	uart_init();
	int md_num;
	iopmp_ops = &riscv_iopmp_ops;

	printf("\nQEMU IOPMP testcase14\n");

	srcmd_fmt = iopmp_ops->get_srcmd_fmt(DEV_IOPMP1);
	mdcfg_fmt = iopmp_ops->get_mdcfg_fmt(DEV_IOPMP1);

	if (srcmd_fmt != 0 || mdcfg_fmt != 0) {
		printf("Failure: This case only supports srcmd_fmt0 mdcfg_fmt0\n");
		while (1);
	}

	md_num = iopmp_ops->get_md_num(DEV_IOPMP1);
	iopmp_k = iopmp_ops->get_md_entry_num(DEV_IOPMP1) + 1;
	printf("IOPMP configuration\n");
	printf("hwcfg0 0x%x\n", iopmp_ops->get_hwcfg(DEV_IOPMP1, 0));
	printf("mdcfg_fmt: %d\n", mdcfg_fmt);
	printf("srcmd_fmt: %d\n", srcmd_fmt);
	printf("number of MD: %d\n", md_num);
	printf("number of RRID: %d\n", iopmp_ops->get_rrid_num(DEV_IOPMP1));
	printf("number of entries: %d\n",  iopmp_ops->get_entry_num(DEV_IOPMP1));
	printf("number of prior entry: %d\n", iopmp_ops->get_prior_entry_num(DEV_IOPMP1));
	if (mdcfg_fmt != 0) {
		printf("k value: %d\n", iopmp_k);
	}
	no_w = iopmp_ops->get_no_w(DEV_IOPMP1);
	printf("no_w %d\n", no_w);

	TEST_RW_Ptr = (unsigned char *)(IOPMP1_REGION_BASE);
	for(int i=0; i<0x100; i++){
		*(TEST_RW_Ptr + i) = i;
	}

	if (mdcfg_fmt == 0) {
		iopmp_k = ENTRY_NUM_PER_MD;
		for(int i=0; i < md_num; i++){
			iopmp_ops->set_mdcfg(DEV_IOPMP1, i, iopmp_k * (i + 1));
		}
	}

	/* Setup IOPMP */
	if (iopmp_k < 3) {
		printf("Failure: The number of IOPMP entry is insufficient for the NAPOT mode demonstration.\n");
		while (1);
	}
	printf("Configure IOPMP entries with NAPOT scheme.\n");

	/* Setup MDs for DEV_IOPMP1 */
	iopmp_ops->srcmd_add_md(DEV_IOPMP1, CPU_RRID, CPU_MD[0]);
	/* 0x0 ~ 0xFFFFFFFF, executable, writable, readable */
	iopmp_ops->napot_config(DEV_IOPMP1, CPU_MD[0] * iopmp_k, (void *)0, 0xFFFFFFFF, ENTRY_CFG_XWR(ENTRY_X_ON, ENTRY_W_ON, ENTRY_R_ON));

	iopmp_ops->srcmd_add_md(DEV_IOPMP1, DMA_RRID, 4);

	int error_reaction_ctrl = (ERR_CFG_CTRL_IE_ENABLE | ERR_CFG_CTRL_RS_DISABLE);
	iopmp_ops->error_reaction(DEV_IOPMP1, error_reaction_ctrl);

	iopmp_irq_init();
	iopmp_ops->enable(DEV_IOPMP1);

	/* Enable the Machine External interrupt */
	set_csr(CSR_MIE, MIP_MEIP);
	/* Enable interrupts in general. */
	set_csr(CSR_MSTATUS, MSTATUS_MIE);

	printf("Stall transcation\n");
	iopmp_ops->stall_transaction(DEV_IOPMP1);
	int cnt = 0;
	int dma_stat;
	printf("DMA memory copy\n");
	dma_mem_copy((long)TEST_RW_Ptr, (long)TEST_RW_Ptr + 0x100, 0x100);

	/* DMA: TEST_RW_Ptr ~ TEST_RW_Ptr + 0x1000, writable, readable */
	iopmp_ops->napot_config(DEV_IOPMP1, 4 * iopmp_k, (void *)TEST_RW_Ptr, 0x1000, ENTRY_CFG_XWR(ENTRY_X_OFF, ENTRY_W_ON, ENTRY_R_ON));

	while (cnt < 1000) {
		dma_stat = dma_get_status();
		if (dma_stat != 0) {
			printf("Failure: transaction is not stalled");
			while (1);
		}
		cnt++;
	}
	printf("resume transcation\n");
	iopmp_ops->resume_transaction(DEV_IOPMP1);
	while (dma_get_status() == 0);
	dma_stat = dma_get_status();
	if (dma_stat != DMA_STATUS_COMPLETE) {
		printf("Failure: DMA status is not complete\n");
		while (1);
	}
	/* Clear DMA status*/
	dma_clear_status(dma_stat);
	if(*(TEST_RW_Ptr + 0x100 + 0x50) != 0x50) {
		printf("Failure: Data error\n");
		while (1);
	}

	printf("Stall transcation\n");
	iopmp_ops->stall_transaction(DEV_IOPMP1);
	cnt = 0;
	printf("DMA memory copy\n");
	dma_mem_copy((long)TEST_RW_Ptr, (long)TEST_RW_Ptr + 0x100, 0x100);

	/* DMA: 0x0 ~ 0xFFFFFFFF, executable, writable, readable */
	iopmp_ops->napot_config(DEV_IOPMP1, 4 * iopmp_k, (void *)TEST_RW_Ptr, 0x1000, ENTRY_CFG_XWR(ENTRY_X_OFF, ENTRY_W_OFF, ENTRY_R_OFF));

	while (cnt < 1000) {
		dma_stat = dma_get_status();
		if (dma_stat != 0) {
			printf("Failure: transaction is not stalled");
			while (1);
		}
		cnt++;
	}
	printf("resume transcation by RRIDSCP\n");
	iopmp_ops->rridscp_op(DEV_IOPMP1, DMA_RRID, RRIDSCP_OP_NO_STALL);
	while (dma_get_status() == 0);
	dma_stat = dma_get_status();
	if (dma_stat != DMA_STATUS_ERROR) {
		printf("Failure: DMA status is not ERROR\n");
		while (1);
	}
	/* Clear DMA status*/
	dma_clear_status(dma_stat);

	printf("IOPMP testcase14 success.\n");
	while(1);
}
