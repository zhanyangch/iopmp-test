#ifndef __PLIC_H__
#define __PLIC_H__
#include <stdint.h>

// #define PLIC_BASE           0x0C000000UL  // Base address of the PLIC
#define PLIC_PRIORITY_BASE  (PLIC_BASE + 0x0000)
#define PLIC_PENDING_BASE   (PLIC_BASE + 0x1000)
#define PLIC_ENABLE_BASE    (PLIC_BASE + 0x2000)
#define PLIC_THRESHOLD      (PLIC_BASE + 0x200000)
#define PLIC_CLAIM_COMPLETE (PLIC_BASE + 0x200004) // For hart 0

__attribute__((always_inline)) static inline void plic_enable_interrupt(uint32_t irq, uint32_t priority) {
    *((volatile uint32_t *)(PLIC_PRIORITY_BASE + irq * 4)) = priority;

    // Enable the interrupt for hart 0
    uint32_t reg_offset = irq / 32;
    uint32_t bit_offset = irq % 32;
    volatile uint32_t *enable_reg = (volatile uint32_t *)(PLIC_ENABLE_BASE + reg_offset * 4);
    *enable_reg |= (1 << bit_offset);
}

__attribute__((always_inline)) static inline void plic_disable_interrupt(uint32_t irq) {
    // Disable the interrupt for hart 0
    uint32_t reg_offset = irq / 32;
    uint32_t bit_offset = irq % 32;
    volatile uint32_t *enable_reg = (volatile uint32_t *)(PLIC_ENABLE_BASE + reg_offset * 4);
    *enable_reg &= ~(1 << bit_offset);
}

__attribute__((always_inline)) static inline uint32_t plic_claim(void) {
    return *((volatile uint32_t *)(PLIC_CLAIM_COMPLETE));
}

__attribute__((always_inline)) static inline void plic_complete(uint32_t irq) {
    *((volatile uint32_t *)(PLIC_CLAIM_COMPLETE)) = irq;
}
#endif