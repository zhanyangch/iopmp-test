#ifndef __CSR_H__
#define __CSR_H__
#include <stdint.h>

#define  CSR_MSTATUS          0x300
#define  CSR_MIE              0x304
#define  CSR_MTVEC            0x305
#define  CSR_MEPC             0x341
#define  CSR_MCAUSE           0x342
#define  CSR_MTVAL            0x343
#define  CSR_MIP              0x344
#define  CSR_MISELECT         0x350
#define  CSR_MIREG            0x351
#define  CSR_MHARTID          0xf14

#if __riscv_xlen == 64
#define MCAUSE_INT            0x8000000000000000UL
#define MCAUSE_CAUSE          0x7FFFFFFFFFFFFFFFUL
#else
#define MCAUSE_INT            0x80000000UL
#define MCAUSE_CAUSE          0x7FFFFFFFUL
#endif

#define MSTATUS_MIE         	0x00000008
#define MIP_MEIP                (1 << 11)

#define TRAP_M_I_ACC_FAULT      1
#define TRAP_M_L_ACC_FAULT      5
#define TRAP_M_S_ACC_FAULT      7


static inline long read_csr(uint32_t csr) {
    long ret;
    __asm__ volatile ("csrr %0, %1" : "=r" (ret) : "i" (csr));
    return ret;
}

static inline long swap_csr(uint32_t csr, long value) {
    long ret;
    __asm__ volatile(
        "csrrw %0, %1, %2"
        : "=r" (ret)
        : "i" (csr), "r" (value)
    );
    return ret;
}

static inline void write_csr(uint32_t csr, long value) {
    __asm__ volatile ("csrw %0, %1" : : "i" (csr), "r" (value));
}

static inline void set_csr(uint32_t csr, long mask) {
    __asm__ volatile ("csrs %0, %1" : : "i" (csr), "r" (mask));
}

static inline void clear_csr(uint32_t csr, long mask) {
    __asm__ volatile ("csrc %0, %1" : : "i" (csr), "r" (mask));
}

#endif