
/*
 * Testcase 8:
 * IOPMP1 checks 0x84000000 ~ 0xFFFFFFFF
 * NO_X=1, get IOPMP instruction error
 * NO_X=0, function execute success
 */

#include "../utility/riscv_iopmp.h"
#include "platform.h"
#include "plic.h"
#include <stdio.h>

#define printf(arg...) do { char str[0x100]; sprintf(str, ##arg); uart_puts(str); } while (0)

#define USE_NAPOT             0

/* RRID list */
#define CPU_RRID              0

#define ARRAY_SIZE            64
#define RAM_BASE              0x80000000

typedef void (*fun_ptr)(void*);

volatile int bus_error_done, iopmp_isr_done, iopmp_read_isr_done, iopmp_fetch_isr_done;
volatile int expect_exception_cause, expect_iopmp_ttype, expect_iopmp_etype;

volatile unsigned char *CPU_EXEC_Ptr;
int chk_x;
int no_x;
int fetch_flag;
int add_perm_done;
int iopmp_k;

RISCV_IOPMP_OPS *iopmp_ops;
extern RISCV_IOPMP_OPS riscv_iopmp_ops;
int CPU_MD[] = {0, 1, 2};


void __attribute__((naked)) dummy_text_context(void* var0)
{
	asm volatile (
		"function_start:"
		"li t0, %[input0];"
		"sw t0, 0(a0);"			/* *var = 7777777 */
		"ret;"
		"function_end:"
		:
		: [input0]"i" (7777777)
		: "t0"
	);
}
void __attribute__((naked)) test_pass(void)
{
	if(no_x) {
		printf("IOPMP testcase8 success.\n");
	} else {
		printf("Failure: should not go here\n");
	}
	while (1);
}

void iopmp_irq_init() {
	plic_enable_interrupt(IRQ_IOPMP_SOURCE, 1);
}

long except_handler(long cause, long epc)
{
	bus_error_done++;

	if (cause != expect_exception_cause) {
		printf("Failure: Get an unexpected exception!\n");
		while (1);
	}

	if (cause == TRAP_M_L_ACC_FAULT) {
		printf("[Load Access Fault]\n");
		printf("Failure: unexcepted Load Access Fault\n");
		while (1);
	}

	if (cause == TRAP_M_S_ACC_FAULT) {
		printf("[Store Access Fault]\n");
		printf("Failure: unexcepted Store Access Fault\n");
		while (1);
	}

	if (cause == TRAP_M_I_ACC_FAULT) {
		printf("[Instruction Access Fault]\n");
		if (fetch_flag) {
			if (chk_x) {
				printf("Jump to another address have excute permission\n");
				epc = (long)test_pass;
			}
		} else {
			printf("Failure: unexcepted Instruction Access Fault\n");
			while (1);
		}
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
	volatile unsigned int function_flag = 0;
	iopmp_ops = &riscv_iopmp_ops;

	printf("\nQEMU IOPMP testcase8\n");

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
	chk_x = iopmp_ops->get_chk_x(DEV_IOPMP1);
	no_x = iopmp_ops->get_no_x(DEV_IOPMP1);
	printf("no_x %d\n", no_x);
	printf("Prepare EXEC region data\n");
	/* IOPMP1 checks (RAM_BASE + 0x4000000) ~ 0xFFFFFFFF in layout 1*/
	CPU_EXEC_Ptr = (unsigned char *)(RAM_BASE + 0x4000000);
	extern unsigned char function_start, function_end;
	char *pDes = (char*)CPU_EXEC_Ptr;
	char *pSrc = (char*)&function_start;
	unsigned long size = (unsigned long)&function_end - (unsigned long)&function_start;

	for (int i = 0; i < size; i++) {
		*pDes++ = *pSrc++;
	}
	fun_ptr fun = (fun_ptr)CPU_EXEC_Ptr;

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

	int error_reaction_ctrl = (ERR_CFG_CTRL_IE_ENABLE | ERR_CFG_CTRL_RS_DISABLE);
	iopmp_ops->error_reaction(DEV_IOPMP1, error_reaction_ctrl);

	iopmp_irq_init();
	iopmp_ops->enable(DEV_IOPMP1);

	/* Enable the Machine External interrupt */
	set_csr(CSR_MIE, MIP_MEIP);
	/* Enable interrupts in general. */
	set_csr(CSR_MSTATUS, MSTATUS_MIE);

	printf("\nTry to fetch from IOPMP1 protect region.\n");
	fetch_flag = 1;
	bus_error_done = 0;
	iopmp_isr_done = 0;
	expect_exception_cause = TRAP_M_I_ACC_FAULT;
	expect_iopmp_ttype = TTYPE_FETCH;
	expect_iopmp_etype = ETYPE_NO_HIT;
	asm volatile("" ::: "memory");
	fun((void*)&function_flag);
	if (no_x) {
		printf("Failure: should not go here\n");
	} else if (function_flag == 7777777) {
		printf("IOPMP testcase8 success.\n");
	} else {
		printf("Failure: function_flag error\n");
	}
	while(1);
}
