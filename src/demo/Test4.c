
/*
 * Testcase 4:
 * The same address with diffrent permission appear in multiple non-prior entries.
 */

#include "../utility/riscv_iopmp.h"
#include "platform.h"
#include "plic.h"
#include <stdio.h>

#define printf(arg...) do { char str[0x100]; sprintf(str, ##arg); uart_puts(str); } while (0)

#define ARRAY_SIZE            64

volatile int bus_error_done, iopmp_isr_done;
volatile int expect_exception_cause;

int iopmp_k;

RISCV_IOPMP_OPS *iopmp_ops;
extern RISCV_IOPMP_OPS riscv_iopmp_ops;
int CPU_MD[] = {0, 1, 2};

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

        volatile unsigned int *CPU_RW_Ptr = (unsigned int *)(RAM_BASE + 0x10000);
        volatile unsigned int *CPU_NO_PERM_Ptr = (unsigned int *)(RAM_BASE + 0x20000);

	printf("\nQEMU IOPMP testcase4\n");

	srcmd_fmt = iopmp_ops->get_srcmd_fmt(DEV_IOPMP0);
	mdcfg_fmt = iopmp_ops->get_mdcfg_fmt(DEV_IOPMP0);

	md_num = iopmp_ops->get_md_num(DEV_IOPMP0);
	printf("IOPMP configuration\n");
	printf("hwcfg0 0x%x\n", iopmp_ops->get_hwcfg(DEV_IOPMP0, 0));
	printf("mdcfg_fmt: %d\n", mdcfg_fmt);
	printf("srcmd_fmt: %d\n", srcmd_fmt);
	printf("number of MD: %d\n", md_num);
	printf("number of RRID: %d\n", iopmp_ops->get_rrid_num(DEV_IOPMP0));
	printf("number of entries: %d\n",  iopmp_ops->get_entry_num(DEV_IOPMP0));
	printf("number of prior entry: %d\n", iopmp_ops->get_prior_entry_num(DEV_IOPMP0));

        if (srcmd_fmt != 0 || mdcfg_fmt != 0) {
		printf("Failure: This case dose not support current format\n");
		while (1);
	}
        if(iopmp_ops->get_prient_prog(DEV_IOPMP0)) {
                printf("Set prior entry to 0\n");
                iopmp_ops->set_prior_entry_num(DEV_IOPMP0,0);
        }

        if(iopmp_ops->get_prior_entry_num(DEV_IOPMP0) > 0) {
                printf("Failure: This case is not for prior_entry\n");
		while (1);
        }

	CPU_MD[2] = md_num - 1;
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
	/* CPU_MD[0] is associated with CPU */
	/* The same address can present in multiple non-prior entries */
	iopmp_ops->srcmd_add_md(DEV_IOPMP0, CPU_RRID, CPU_MD[0]);
	/* CPU_MD[0] region 1: CPU_RW_Ptr ~ (CPU_RW_Ptr + ARRAY_SIZE), no executable, no writable, readable */
	iopmp_ops->napot_config(DEV_IOPMP0, CPU_MD[0] * iopmp_k + 1, (void *)CPU_RW_Ptr, ARRAY_SIZE, ENTRY_CFG_XWR(ENTRY_X_OFF, ENTRY_W_OFF, ENTRY_R_ON));

	iopmp_ops->srcmd_add_md(DEV_IOPMP0, CPU_RRID, CPU_MD[1]);
	/* CPU_MD[1] region 1: CPU_RW_Ptr ~ (CPU_RW_Ptr + ARRAY_SIZE), no executable, writable, no readable */
	iopmp_ops->napot_config(DEV_IOPMP0, CPU_MD[1] * iopmp_k, (void *)CPU_RW_Ptr, ARRAY_SIZE, ENTRY_CFG_XWR(ENTRY_X_OFF, ENTRY_W_ON, ENTRY_R_OFF));

	/* CPU_MD[2] is associated with CPU */
	iopmp_ops->srcmd_add_md(DEV_IOPMP0, CPU_RRID, CPU_MD[2]);
	/* CPU_MD[2] region 1 : 0x0 ~ 0x8000000, executable, writable, readable */
	iopmp_ops->napot_config(DEV_IOPMP0, CPU_MD[2] * iopmp_k, (void *)0, RAM_BASE, ENTRY_CFG_XWR(ENTRY_X_ON, ENTRY_W_ON, ENTRY_R_ON));
        /* CPU_MD[2] region 1 : 0x8000000 ~ 0x8000000 + 0x10000, executable, writable, readable */
        iopmp_ops->napot_config(DEV_IOPMP0, CPU_MD[2] * iopmp_k+1, (void *)RAM_BASE, 0x10000, ENTRY_CFG_XWR(ENTRY_X_ON, ENTRY_W_ON, ENTRY_R_ON));
	/* STACK : 0x8300000 ~ 0x8400000, executable, writable, readable */
	iopmp_ops->napot_config(DEV_IOPMP0, CPU_MD[2] * iopmp_k+2, (void *)(STACK_BASE- STACK_SIZE), STACK_SIZE, ENTRY_CFG_XWR(ENTRY_X_ON, ENTRY_W_ON, ENTRY_R_ON));

	int error_reaction_ctrl = (ERR_CFG_CTRL_RS_DISABLE | ERR_CFG_CTRL_IE_ENABLE);
	iopmp_ops->error_reaction(DEV_IOPMP0, error_reaction_ctrl);

	iopmp_irq_init();
	iopmp_ops->enable(DEV_IOPMP0);

	/* Enable the Machine External interrupt */
	set_csr(CSR_MIE, MIP_MEIP);
	/* Enable interrupts in general. */
	set_csr(CSR_MSTATUS, MSTATUS_MIE);

        printf("\nTry to write data to RW region.\n");
	asm volatile("" ::: "memory");
	*CPU_RW_Ptr = 0x7777CCCC;

	printf("\nTry to read data from RW region.\n");
	asm volatile("" ::: "memory");
	volatile int data = *CPU_RW_Ptr;
	if (data != 0x7777CCCC) {
		printf("Failure: Data %x\n", data);
		while (1);
	}

        printf("\nTry to read data from No permssion region.\n");
	bus_error_done = 0;
	iopmp_isr_done = 0;
	expect_exception_cause = TRAP_M_L_ACC_FAULT;
	asm volatile("" ::: "memory");
        data = 0;
	data = *CPU_NO_PERM_Ptr;
        while (!bus_error_done || !iopmp_isr_done);
	if (data == 0x7777CCCC) {
		printf("Failure: Data %x\n", data);
		while (1);
	}

	printf("IOPMP testcase4 success.\n");
	while(1);
}
