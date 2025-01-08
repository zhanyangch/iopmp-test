	.section .init
	.option push
	.option norelax

	.global ___init

___init:

	li sp, 0x84000000
	la t0, trap_entry
	csrw mtvec, t0
	j _start

	.option pop
	.section .text

	.weak trap_entry
	.align 2

trap_entry:
	addi sp, sp, -16*8
	sd ra,  0*8(sp)
	sd t0,  1*8(sp)
	sd t1,  2*8(sp)
	sd t2,  3*8(sp)
	sd a0,  4*8(sp)
	sd a1,  5*8(sp)
	sd a2,  6*8(sp)
	sd a3,  7*8(sp)
	sd a4,  8*8(sp)
	sd a5,  9*8(sp)
	sd a6, 10*8(sp)
	sd a7, 11*8(sp)
	sd t3, 12*8(sp)
	sd t4, 13*8(sp)
	sd t5, 14*8(sp)
	sd t6, 15*8(sp)

	csrr a0, mcause
	bgez a0, not_irq

	call interrupt_entry

resd:
	ld ra,  0*8(sp)
	ld t0,  1*8(sp)
	ld t1,  2*8(sp)
	ld t2,  3*8(sp)
	ld a0,  4*8(sp)
	ld a1,  5*8(sp)
	ld a2,  6*8(sp)
	ld a3,  7*8(sp)
	ld a4,  8*8(sp)
	ld a5,  9*8(sp)
	ld a6, 10*8(sp)
	ld a7, 11*8(sp)
	ld t3, 12*8(sp)
	ld t4, 13*8(sp)
	ld t5, 14*8(sp)
	ld t6, 15*8(sp)
	addi sp, sp, 16*8

	mret

not_irq:
	/* Exception handler */
	call except_entry

	j resd