
/*
 * Testcase 10:
 * Cascading IOPMP0 (0x0~ 0xFFFFFFFF) -> IOPMP1 (0x0~ 0xFFFFFFFF)
 * Program IOPMP0 rrid_transl = TRANSL_RRID
 * CPU have all permissions in IOPMP0
 * IOPMP1 gets transaction with TRANSL_RRID
 * Config and test the permissions in IOPMP1
 */

#include "../utility/riscv_iopmp.h"
#include "platform.h"
#include "plic.h"
#include <stdio.h>

#define printf(arg...) do { char str[0x100]; sprintf(str, ##arg); uart_puts(str); } while (0)

#define USE_NAPOT             0

/* RRID list */
#define CPU_RRID              0
#define TRANSL_RRID           5

#define ARRAY_SIZE            64
#define RAM_BASE              0x80000000

typedef void (*fun_ptr)(void*);

volatile int bus_error_done, iopmp_isr_done, iopmp_read_isr_done, iopmp_fetch_isr_done;
volatile int expect_exception_cause, expect_iopmp_ttype, expect_iopmp_etype;

volatile unsigned char CPU_RO_Array[ARRAY_SIZE] __attribute__ ((aligned(ARRAY_SIZE)));
volatile unsigned char CPU_WO_Array[ARRAY_SIZE] __attribute__ ((aligned(ARRAY_SIZE)));

volatile unsigned char *CPU_Write_Ptr;
int no_w;
int fetch_flag;
int add_perm_done;
int iopmp_k;

RISCV_IOPMP_OPS *iopmp_ops;
extern RISCV_IOPMP_OPS riscv_iopmp_ops;
int CPU_MD[] = {0, 1, 2};
int TRANSL_MD[] = {0, 3, 4};

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
	if (iopmp_ops->get_irq_pending(DEV_IOPMP0)) {
		printf("Failure: Should not get IOPMP0 error in this test\n");
		while(1);
	}
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

	printf("\nQEMU IOPMP testcase10\n");

	srcmd_fmt = iopmp_ops->get_srcmd_fmt(DEV_IOPMP0);
	mdcfg_fmt = iopmp_ops->get_mdcfg_fmt(DEV_IOPMP0);

	if (srcmd_fmt != 0 || mdcfg_fmt != 0) {
		printf("Failure: This case only supports srcmd_fmt0 mdcfg_fmt0\n");
		while (1);
	}

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
			iopmp_ops->set_mdcfg(DEV_IOPMP1, i, iopmp_k * (i + 1));
		}
	}

	/* Prepare the data */
	*(volatile unsigned int *)&CPU_RO_Array[0] = 0x5555AAAA;
	*(volatile unsigned int *)&CPU_WO_Array[0] = 0x7777CCCC;

	printf("Set IOPMP0 RRID_TRANSL\n");
	iopmp_ops->set_rrid_transl(DEV_IOPMP0, TRANSL_RRID);

	printf("Configure IOPMP entries\n");

	/* Setup MDs for DEV_IOPMP0 */
	iopmp_ops->srcmd_add_md(DEV_IOPMP0, CPU_RRID, CPU_MD[0]);
	/* 0x0 ~ 0xFFFFFFFF, executable, writable, readable */
	iopmp_ops->napot_config(DEV_IOPMP0, CPU_MD[0] * iopmp_k, (void *)0, 0xFFFFFFFF, ENTRY_CFG_XWR(ENTRY_X_ON, ENTRY_W_ON, ENTRY_R_ON));

	/* Setup MDs for DEV_IOPMP1 */
	iopmp_ops->srcmd_add_md(DEV_IOPMP1, TRANSL_RRID, TRANSL_MD[0]);
	/* TRANSL_MD[0] region 1: CPU_RO_Array ~ (CPU_RO_Array + ARRAY_SIZE), no executable, no writable, readable */
	iopmp_ops->off_config(DEV_IOPMP1, TRANSL_MD[0] * iopmp_k, (void *)CPU_RO_Array);
	iopmp_ops->tor_config(DEV_IOPMP1, TRANSL_MD[0] * iopmp_k + 1, (void *)&CPU_RO_Array[ARRAY_SIZE], ENTRY_CFG_XWR(ENTRY_X_OFF, ENTRY_W_OFF, ENTRY_R_ON));

	iopmp_ops->srcmd_add_md(DEV_IOPMP1, TRANSL_RRID, TRANSL_MD[1]);
	/* TRANSL_MD[1] region 1: CPU_WO_Array ~ (CPU_WO_Array + ARRAY_SIZE), no executable, writable, no readable */
	iopmp_ops->off_config(DEV_IOPMP1, TRANSL_MD[1] * iopmp_k, (void *)CPU_WO_Array);
	iopmp_ops->tor_config(DEV_IOPMP1, TRANSL_MD[1] * iopmp_k + 1, (void *)&CPU_WO_Array[ARRAY_SIZE], ENTRY_CFG_XWR(ENTRY_X_OFF, ENTRY_W_ON, ENTRY_R_OFF));

	iopmp_ops->srcmd_add_md(DEV_IOPMP1, TRANSL_RRID, TRANSL_MD[2]);
	/* TRANSL_MD[2] region 1 : 0x0 ~ 0xFFFFFFFF, executable, writable, readable */
	iopmp_ops->off_config(DEV_IOPMP1, TRANSL_MD[2] * iopmp_k, 0);
	iopmp_ops->tor_config(DEV_IOPMP1, TRANSL_MD[2] * iopmp_k + 1, (void *)0xFFFFFFFF, ENTRY_CFG_XWR(ENTRY_X_ON, ENTRY_W_ON, ENTRY_R_ON));

	int error_reaction_ctrl = (ERR_CFG_CTRL_IE_ENABLE | ERR_CFG_CTRL_RS_DISABLE);
	iopmp_ops->error_reaction(DEV_IOPMP0, error_reaction_ctrl);
	iopmp_ops->error_reaction(DEV_IOPMP1, error_reaction_ctrl);

	iopmp_irq_init();
	iopmp_ops->enable(DEV_IOPMP0);
	iopmp_ops->enable(DEV_IOPMP1);

	/* Enable the Machine External interrupt */
	set_csr(CSR_MIE, MIP_MEIP);
	/* Enable interrupts in general. */
	set_csr(CSR_MSTATUS, MSTATUS_MIE);

	printf("\nTry to read data from WO region.\n");
	bus_error_done = 0;
	iopmp_isr_done = 0;
	expect_exception_cause = TRAP_M_L_ACC_FAULT;
	asm volatile("" ::: "memory");
	unsigned int data = *(volatile unsigned int *)CPU_WO_Array;
	while (!bus_error_done || !iopmp_isr_done);
	if (data == 0x7777CCCC) {
		printf("Failure: WO Data %x\n", data);
		while (1);
	}
	printf("\nTry to write data to RO region.\n");
	bus_error_done = 0;
	iopmp_isr_done = 0;
	expect_exception_cause = TRAP_M_S_ACC_FAULT;
	asm volatile("" ::: "memory");
	*(volatile unsigned int *)&CPU_RO_Array[0] = 0x7777CCCC;
	while (!bus_error_done || !iopmp_isr_done);
	if (*(volatile unsigned int *)&CPU_RO_Array == 0x7777CCCC) {
		printf("Failure: RO Data %x\n", data);
		while (1);
	}

	printf("IOPMP testcase10 success.\n");
	while(1);
}
