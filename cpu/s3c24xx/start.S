/*
 *  cpu/s3c24xx/start.S
 *
 *  U-Boot - Startup Code for S3C24XX
 *
 *  Copyright (c) 2006,2007,2008  Samsung Electronics
 *  All rights reserved.
 *
 *  Based on cpu/arm926ejs/start.S
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * $Id: start.S,v 1.7 2008/04/22 07:31:41 boyko Exp $
 */

/*
 *  mod by sc.suh@samsung.com
 *
 *  Our U-Boot Memory Map with static mmu_table
 *                                      (offset)
 *       --------------------------     0x04000000
 *       |     Stack     (512KB)  |
 *       --------------------------     0x03f80000
 *       |     Heap       (1MB)   |
 *       --------------------------     0x03e80000
 *       |     IRQ Stack  (4KB)   | <------------------------ if exists
 *       --------------------------     0x03e70000
 *       |     FIQ Stack  (4KB)   | <------------------------ if exists
 *       --------------------------     0x03e60000
 *       |     GBL       (128B)   |
 *       --------------------------     0x03exxxxx
 *       |     BSS and Reserved   |
 *       --------------------------     0x03e40000
 *       |     U-Boot    (256KB)  |
 *       --------------------------     0x03e00000
 *
 *  Our U-Boot Memory Map with run-time mmu_table
 *                                      (offset)
 *       --------------------------     0x04000000
 *       |     Stack     (256KB)  |
 *       --------------------------     0x03fc0000
 *       |     Heap       (1MB)   |
 *       --------------------------     0x03ec0000
 *       |     IRQ Stack  (4KB)   | <------------------------ if exists
 *       --------------------------     0x03ec0000
 *       |     FIQ Stack  (4KB)   | <------------------------ if exists
 *       --------------------------     0x03eb0000
 *       |     GBL       (128B)   |
 *       --------------------------     0x03exxxxx
 *       |     MMU Table (16KB)   |
 *       --------------------------     0x03e9c000
 *       |     BSS and Reserved   |
 *       --------------------------     0x03e40000
 *       |     U-Boot    (256KB)  |
 *       --------------------------     0x03e00000
 */

#include <config.h>
#include <version.h>
#ifdef CONFIG_ENABLE_MMU
#include <asm/proc/domain.h>
#endif

#ifndef CONFIG_ENABLE_MMU
#ifndef CFG_PHY_UBOOT_BASE
#define CFG_PHY_UBOOT_BASE	CFG_UBOOT_BASE
#endif
#endif


/*
 *************************************************************************
 *
 * Jump vector table as in table 3.1 in [1]
 *
 *************************************************************************
 */


.globl _start
_start:
	b	reset
	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _not_used
	ldr	pc, _irq
	ldr	pc, _fiq

_undefined_instruction:
	.word undefined_instruction
_software_interrupt:
	.word software_interrupt
_prefetch_abort:
	.word prefetch_abort
_data_abort:
	.word data_abort
_not_used:
	.word not_used
_irq:
	.word irq
_fiq:
	.word fiq

	.balignl 16,0xdeadbeef


/*
 *************************************************************************
 *
 * Startup Code (reset vector)
 *
 * do important init only if we don't start from memory!
 * setup Memory and board specific bits prior to relocation.
 * relocate armboot to ram
 * setup stack
 *
 *************************************************************************
 */

_TEXT_BASE:
	.word	TEXT_BASE

/*
 * Below variable is very important because we use MMU in U-Boot.
 * Without it, we cannot run code correctly before MMU is ON.
 * by scsuh.
 */
_TEXT_PHY_BASE:
	.word	CFG_PHY_UBOOT_BASE

.globl _armboot_start
_armboot_start:
	.word _start

/*
 * These are defined in the board-specific linker script.
 */
.globl _bss_start
_bss_start:
	.word __bss_start

.globl _bss_end
_bss_end:
	.word _end

#ifdef CONFIG_USE_IRQ
/* IRQ stack memory (calculated at run-time) */
.globl IRQ_STACK_START
IRQ_STACK_START:
	.word	0x0badc0de

/* IRQ stack memory (calculated at run-time) */
.globl FIQ_STACK_START
FIQ_STACK_START:
	.word 0x0badc0de
#endif


/*
 * the actual reset code
 */

reset:
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0xd3
	msr	cpsr,r0

#if defined(CONFIG_S3C2443) ||defined(CONFIG_S3C2450) || defined(CONFIG_S3C2416)
	/*
	 * Retention IO power will be turen off whel sleep mode,
	 * but, when wakeup process starts, User should write '1'
	 * produce power on retention IO. PM check
	 */
	ldr	r0, =0x4c00006c
	ldr	r1, =0x4c000064
	ldr	r2, [r0]
	tst 	r2, #0x8

	ldreq	r2, [r1]
	orreq	r2, r2, #0x10000  /* (1<<16) */
	streq	r2, [r1]
#endif

	/*
	 * we do sys-critical inits only at reboot,
	 * not when booting from ram!
	 */
cpu_init_crit:
	/*
	 * flush v4 I/D caches
	 */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0	/* flush v3/v4 cache */
	mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB */

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002300	/* clear bits 13, 9:8 (--V- --RS) */
	bic	r0, r0, #0x00000087	/* clear bits 7, 2:0 (B--- -CAM) */
	orr	r0, r0, #0x00000002	/* set bit 2 (A) Align */
	orr	r0, r0, #0x00001000	/* set bit 12 (I) I-Cache */
	mcr	p15, 0, r0, c1, c0, 0

#ifdef CONFIG_ONENAND
	/*
	 * With this check, we can change boot sequence as we want in run-time.
	 * XXX: must modify to use "swp" not "ldr and str".
	 */
check_onenand_boot:
	mov	r0, #0x1e400	/* check OneNAND via start buffer reg */
	ldr	r1, =0xfffffffe	/* very tweaky and may occur bugs */
	ldr	r2, =0x00000f02
	ldr	r3, [r0]
	str	r1, [r0]
	ldr	r1, [r0]
	str	r3, [r0]
	cmp	r1, r2
	bne	1024f

	/* OneNAND is detected as boot device
	 * So, load <0x400 ~ 0xc00> to DataRam0
	 */
fill_onenand_dr:
	mov	r2, #0
	ldr	r3, =0x0001e200
	ldr	r5, =0x0002	/* 0x400 ~ 0x800 */
	ldr	r6, =0x0001e400
	ldr	r7, =0x0802	/* fill 1KB in DR0 */

100:	strh	r2, [r3]	/* block = 0, data buffer = 0 */
	strh	r2, [r3, #0x2]	/* block = 0, data buffer = 0 */
	strh	r5, [r3, #0xe]	/* set page, sector addr */
	strh	r7, [r6]	/* set start buffer and count */
	strh	r2, [r6, #0x82]	/* reset int status */
	strh	r2, [r6, #0x40]	/* send LOAD command */
1:	ldrh	r8, [r6, #0x82]	/* check int status */
	tst	r8, #(1<<15)
	beq	1b
	tst	r5, #4;
	ldr	r5, =0x0004	/* 0x800 ~ 0xc00 */
	add	r7, r7, #0x0200	/* fill next 1KB 0x0a02 */
	beq	100b
	b	1024f

	.ltorg			/* without it, variables may go too far. */

1024:
#endif

	/*
	 * Go setup Memory and board specific bits prior to relocation.
	 */
	bl	lowlevel_init	/* go setup pll,mux,memory */

#ifdef CONFIG_S3C2442
#ifdef CONFIG_PM
	@ Check if this is a wake-up from sleep
	ldr r1, PMST_ADDR
	ldr r0, [r1]
	tst r0, #0x2 @ PMST_SMR
	bne WakeupStart
#endif
#endif
	/* when we already run in ram, we don't need to relocate U-Boot.
	 * and actually, memory controller must be configured before U-Boot
	 * is running in ram.
	 */
check_boot_device:
	ldr	r0, =0xff000fff
	bic	r1, pc, r0		/* r0 <- current base addr of code */
	ldr	r2, _TEXT_BASE		/* r1 <- original base addr in ram */
	bic	r2, r2, r0		/* r0 <- current base addr of code */
	cmp     r1, r2                  /* compare r0, r1                  */
	beq     after_copy		/* r0 == r1 then skip flash copy   */

#if 1
	ldr	sp, _TEXT_PHY_BASE	/* setup temp stack pointer */
	sub	sp, sp, #12
	mov	fp, #0			/* no previous frame, so fp=0 */
	bl	copy_auto_uboot_to_ram
	cmp	r0, #0xffffffff
	bne	after_copy
#endif

	/* check boot device is nand or nor */
	ldr	r0, =0x00000000
	ldr	r3, [r0]
	ldr	r1, =0xfffffffe
	str	r1, [r0]

	ldr	r2, [r0]
	str	r3, [r0]
	cmp	r1, r2

	beq	nand_copy

#ifdef CONFIG_ONENAND
	ldr	r3, [r0, #0x400]
	ldr	r1, =0xfffffffe
	str	r1, [r0, #0x400]

	ldr	r2, [r0, #0x400]
	str	r3, [r0, #0x400]
	cmp	r1, r2
	beq	jump_to_onenand
#endif

	/* nor copy */
relocate:				/* relocate U-Boot to RAM	    */
	adr	r0, _start		/* r0 <- current position of code   */
@	ldr	r1, _TEXT_BASE
	ldr	r1, _TEXT_PHY_BASE	/* r1 <- destination                */

	ldr	r2, _armboot_start
	ldr	r3, _bss_start
	sub	r2, r3, r2		/* r2 <- size of armboot            */
	add	r2, r0, r2		/* r2 <- source end address         */

copy_loop:
	ldmia	r0!, {r3-r10}		/* copy from source address [r0]    */
	stmia	r1!, {r3-r10}		/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end addreee [r2]    */
	ble	copy_loop
	b	after_copy

nand_copy:
	mov	r0, #0x1000
	bl	copy_from_nand

#ifdef CONFIG_ONENAND
	b	after_copy

jump_to_onenand:
	bl	temp_copy_onenand
onenand_copy:
	mov	r0, #0x400
	bl	copy_from_nand
#endif

after_copy:
#ifdef CONFIG_ENABLE_MMU
#ifdef CONFIG_BUILD_MMU_TABLE_RUNTIME
	ldr	sp, _TEXT_PHY_BASE	/* setup temp stack pointer */
	sub	sp, sp, #12
	mov	fp, #0			/* no previous frame, so fp=0 */
	bl	make_mmu_table
#endif
enable_mmu:
	/* enable domain access */
	ldr	r5, =0x0000ffff
	mcr	p15, 0, r5, c3, c0, 0		@ load domain access register

	/* Set the TTB register */
#ifdef CONFIG_BUILD_MMU_TABLE_RUNTIME
	ldr	r1, =MMU_TABLE_BASE
#else
	ldr	r0, _mmu_table_base
	ldr	r1, =CFG_PHY_UBOOT_BASE
	ldr	r2, =0xfff00000
	bic	r0, r0, r2
	orr	r1, r0, r1
#endif
	mcr	p15, 0, r1, c2, c0, 0

	/* Enable the MMU */
mmu_on:
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #1			/* Set CR_M to enable MMU */
	mcr	p15, 0, r0, c1, c0, 0
	nop
	nop
	nop
	nop
#endif

	/* Set up the stack						    */
stack_setup:
#ifdef CONFIG_MEMORY_UPPER_CODE
	ldr	sp, =(CFG_UBOOT_BASE + CFG_UBOOT_SIZE - 0xc)
#else
	ldr	r0, _TEXT_BASE		/* upper 128 KiB: relocated uboot   */
	sub	r0, r0, #CFG_MALLOC_LEN	/* malloc area                      */
	sub	r0, r0, #CFG_GBL_DATA_SIZE /* bdinfo                        */
#ifdef CONFIG_USE_IRQ
	sub	r0, r0, #(CONFIG_STACKSIZE_IRQ+CONFIG_STACKSIZE_FIQ)
#endif
	sub	sp, r0, #12		/* leave 3 words for abort-stack    */

#endif

clear_bss:
	ldr	r0, _bss_start		/* find start of bss segment        */
	ldr	r1, _bss_end		/* stop here                        */
	mov 	r2, #0x00000000		/* clear                            */

clbss_l:str	r2, [r0]		/* clear loop...                    */
	add	r0, r0, #4
	cmp	r0, r1
	ble	clbss_l

	ldr	pc, _start_armboot

_start_armboot:
	.word start_armboot

#ifdef CONFIG_ENABLE_MMU
#ifndef CONFIG_BUILD_MMU_TABLE_RUNTIME
_mmu_table_base:
	.word mmu_table
#endif
#endif

#ifdef CONFIG_ONENAND
temp_copy_onenand:
	adr	r0, _start		/* r0 <- current position of code   */
	ldr	r1, _TEXT_PHY_BASE	/* test if we run from flash or RAM */
	ldr	r2, =0xbff

1:	ldmia	r0!, {r3-r10}		/* copy from source address [r0]    */
	stmia	r1!, {r3-r10}		/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end addreee [r2]    */
	ble	1b

	adr	r0, onenand_copy
	ldr	r1, _TEXT_PHY_BASE
	add	r0, r0, r1
	mov	pc, r0

	.ltorg
#endif

/*
 * copy U-Boot to SDRAM and jump to ram (from NAND or OneNAND)
 * r0: size to be compared
 */
	.globl copy_from_nand
copy_from_nand:
	mov	r10, lr		/* save return address */

	mov	r9, r0
	/* get ready to call C functions */
	ldr	sp, _TEXT_PHY_BASE	/* setup temp stack pointer */
	sub	sp, sp, #12
	mov	fp, #0			/* no previous frame, so fp=0 */

#ifdef CONFIG_ONENAND
	cmp	r9, #0x1000
	bne	2f
	bl	copy_uboot_to_ram
	b	3f
2:	bl	onenand_cp
#else
	mov	r9, #0x1000
	bl	copy_uboot_to_ram
#endif
3:	tst 	r0, #0x0
	bne	copy_failed

#if defined(CONFIG_S3C2450) || defined(CONFIG_S3C2416)

/* Confirm Booting Status NAND Booting or iROM NAND*/

	ldr	r6, =0x40008000
	ldr	r7, =0x24564236
	swp	r8, r7, [r6]
	swp	r5, r8, [r6]
	cmp	r7, r5

/* If compare value is same between r7 and r5, Booting Device is iROM */

	beq	444f

	mov	r0, #0		/* NAND Booting */
	b	555f
444:
	mov	r0, #0x40000000	/* iROM booting */

#else
	mov	r0, #0

#endif

555:
	ldr	r1, _TEXT_PHY_BASE
1:	ldr	r3, [r0], #4
	ldr	r4, [r1], #4
	teq	r3, r4
	bne	compare_failed	/* not matched */
	subs	r9, r9, #4
	bne	1b

4:	mov	lr, r10		/* all is OK */
	mov	pc, lr

copy_failed:
	nop			/* copy from nand failed */
	b	copy_failed
compare_failed:
	nop			/* compare failed */
	b	compare_failed

/*
 * we assume that cache operation is done before. (eg. cleanup_before_linux())
 * actually, we don't need to do anything about cache if not use d-cache in U-Boot
 * So, in this function we clean only MMU. by scsuh
 *
 * void	theLastJump(void *kernel, int arch_num, uint boot_params);
 */
#ifdef CONFIG_ENABLE_MMU
	.globl theLastJump
theLastJump:
	mov	r9, r0
	ldr	r3, =0xfff00000
	ldr	r4, _TEXT_PHY_BASE
	adr	r5, phy_last_jump
	bic	r5, r5, r3
	orr	r5, r5, r4
	mov	pc, r5
phy_last_jump:
	/*
	 * disable MMU stuff
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002300	/* clear bits 13, 9:8 (--V- --RS) */
	bic	r0, r0, #0x00000087	/* clear bits 7, 2:0 (B--- -CAM) */
	orr	r0, r0, #0x00000002	/* set bit 2 (A) Align */
	orr	r0, r0, #0x00001000	/* set bit 12 (I) I-Cache */
	mcr	p15, 0, r0, c1, c0, 0

	mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB */

	mov	r0, #0
	mov	pc, r9
#endif

/*
 *************************************************************************
 *
 * Interrupt handling
 *
 *************************************************************************
 */

@
@ IRQ stack frame.
@
#define S_FRAME_SIZE	72

#define S_OLD_R0	68
#define S_PSR		64
#define S_PC		60
#define S_LR		56
#define S_SP		52

#define S_IP		48
#define S_FP		44
#define S_R10		40
#define S_R9		36
#define S_R8		32
#define S_R7		28
#define S_R6		24
#define S_R5		20
#define S_R4		16
#define S_R3		12
#define S_R2		8
#define S_R1		4
#define S_R0		0

#define MODE_SVC	0x13
#define I_BIT		0x80

/*
 * use bad_save_user_regs for abort/prefetch/undef/swi ...
 * use irq_save_user_regs / irq_restore_user_regs for IRQ/FIQ handling
 */

	.macro	bad_save_user_regs
	@ carve out a frame on current user stack
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}	@ Save user registers (now in svc mode) r0-r12

	ldr	r2, _armboot_start
	sub	r2, r2, #(CONFIG_STACKSIZE+CFG_MALLOC_LEN)
	sub	r2, r2, #(CFG_GBL_DATA_SIZE+8)  @ set base 2 words into abort stack
	@ get values for "aborted" pc and cpsr (into parm regs)
	ldmia	r2, {r2 - r3}
	add	r0, sp, #S_FRAME_SIZE		@ grab pointer to old stack
	add	r5, sp, #S_SP
	mov	r1, lr
	stmia	r5, {r0 - r3}	@ save sp_SVC, lr_SVC, pc, cpsr
	mov	r0, sp		@ save current stack into r0 (param register)
	.endm

	.macro	irq_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0-r12
	@ !!!! R8 NEEDS to be saved !!!! a reserved stack spot would be good.
	add	r8, sp, #S_PC
	stmdb	r8, {sp, lr}^		@ Calling SP, LR
	str	lr, [r8, #0]		@ Save calling PC
	mrs	r6, spsr
	str	r6, [r8, #4]		@ Save CPSR
	str	r0, [r8, #8]		@ Save OLD_R0
	mov	r0, sp
	.endm

	.macro	irq_restore_user_regs
	ldmia	sp, {r0 - lr}^			@ Calling r0 - lr
	mov	r0, r0
	ldr	lr, [sp, #S_PC]			@ Get PC
	add	sp, sp, #S_FRAME_SIZE
	subs	pc, lr, #4		@ return & move spsr_svc into cpsr
	.endm

	.macro get_bad_stack
	ldr	r13, _armboot_start		@ setup our mode stack
	sub	r13, r13, #(CONFIG_STACKSIZE+CFG_MALLOC_LEN)
	sub	r13, r13, #(CFG_GBL_DATA_SIZE+8) @ reserved a couple spots in abort stack

	str	lr, [r13]	@ save caller lr in position 0 of saved stack
	mrs	lr, spsr	@ get the spsr
	str	lr, [r13, #4]	@ save spsr in position 1 of saved stack
	mov	r13, #MODE_SVC	@ prepare SVC-Mode
	@ msr	spsr_c, r13
	msr	spsr, r13	@ switch modes, make sure moves will execute
	mov	lr, pc		@ capture return pc
	movs	pc, lr		@ jump to next instruction & switch modes.
	.endm

	.macro get_irq_stack			@ setup IRQ stack
	ldr	sp, IRQ_STACK_START
	.endm

	.macro get_fiq_stack			@ setup FIQ stack
	ldr	sp, FIQ_STACK_START
	.endm

/*
 * exception handlers
 */
	.align  5
undefined_instruction:
@	get_bad_stack
@	bad_save_user_regs
	bl	do_undefined_instruction

	.align	5
software_interrupt:
@	get_bad_stack
@	bad_save_user_regs
	bl	do_software_interrupt

	.align	5
prefetch_abort:
@	get_bad_stack
@	bad_save_user_regs
	bl	do_prefetch_abort

	.align	5
data_abort:
	get_bad_stack
	bad_save_user_regs
	bl	do_data_abort

	.align	5
not_used:
@	get_bad_stack
@	bad_save_user_regs
	bl	do_not_used

#ifdef CONFIG_USE_IRQ

	.align	5
irq:
	get_irq_stack
	irq_save_user_regs
	bl 	do_irq
	irq_restore_user_regs

	.align	5
fiq:
	get_fiq_stack
	/* someone ought to write a more effiction fiq_save_user_regs */
	irq_save_user_regs
	bl 	do_fiq
	irq_restore_user_regs

#else

	.align	5
irq:
@	get_bad_stack
@	bad_save_user_regs
	bl	do_irq

	.align	5
fiq:
@	get_bad_stack
@	bad_save_user_regs
	bl	do_fiq

#endif

#ifdef CONFIG_PM
	.align 4
PMCTL1_ADDR:	.long 0x56000080
PMST_ADDR:	.long 0x560000B4
PMSR0_ADDR:	.long 0x560000B8
GPBCON:		.long 0x56000010
GPBDAT:		.long 0x56000014
GPFCON_reg:	.long 0x56000050
GPFDAT_reg:	.long 0x56000054

	.align 5
sleep_setting:
@ prepare the SDRAM self-refresh mode
	ldr r0, =0x48000024	@ REFRESH Register
	ldr r1, [r0]
	orr r1, r1,#(1<<22) @ self-refresh bit set

@ prepare MISCCR[19:17]=111b to make SDRAM signals(SCLK0,SCLK1,SCKE) protected
	ldr r2,=0x56000080	@ MISCCR Register
	ldr r3,[r2]
	orr r3,r3,#((1<<17)|(1<<18)|(1<<19))

@ prepare the Power_Off mode bit in CLKCON Register
	ldr r4,=0x4c00000c	@ CLKCON Register
	ldr r5,=(1<<3)
	b   set_sdram_refresh

	.align 5
set_sdram_refresh:
	str r1,[r0]             @ SDRAM self-refresh enable

@ wait until SDRAM into self-refresh
	mov r1, #64
1:  subs    r1, r1, #1
	bne 1b

@ set the MISCCR & CLKCON register for power off
	str r3,[r2]
	str r5,[r4]
	nop                 @ waiting for power off
	nop
	nop
	b   .

	.align 5
WakeupStart:
	@ Clear sleep reset bit
	ldr r0, PMST_ADDR
	mov r1, #(1<<1) @ PMST_SMR
	str r1, [r0]

	@ Release the SDRAM signal protections
	ldr r0, PMCTL1_ADDR
	ldr r1, [r0]
	bic r1, r1, #((1<<17)|(1<<18)|(1<<19))  @ (SCLKE | SCLK1 | SCLK0)
	str r1, [r0]

	@ Max1718_Set();  @for case 135 i.e 300MHz operation
@	GPBCON = (GPBCON & ~((3 << 20) | (3 << 16) | (3 << 14))) | (1 << 20) | (1 << 16) | (1 << 14);
	ldr r1, GPBCON
	ldr r0, [r1]
	bic r0, r0, #( (3 << 20) | (3 << 16) | (3 << 14) )
	orr r0, r0, #( (1 << 20) | (1 << 16) | (1 << 14) )
	str r0, [r1]
	// GPB7, 8, 10 : Output

@	GPFCON = (GPFCON & ~(0xff << 8)) | (0x55 << 8);	// GPF4~7: Output , shared with LED4~7
	ldr r1, GPFCON_reg
	ldr r0, [r1]
	bic r0, r0, #( (0xff << 8) )
	orr r0, r0, #( (0x55 << 8) )
	str r0, [r1]

@	GPBDAT = (GPBDAT & ~(1 << 7)) | (0 << 7);   //D4
	ldr r1, GPBDAT
	ldr r0, [r1]
	bic r0, r0, #( (1 << 7) )
	orr r0, r0, #( (0 << 7) )
	str r0, [r1]

@	GPFDAT = (GPFDAT & ~(0xf << 4)) | (1 << 7) | (0 << 6) | (0 << 5) | (0 << 4);    //D3~0
	ldr r1, GPFDAT_reg
	ldr r0, [r1]
	bic r0, r0, #( (0xf << 4) )
	orr r0, r0, #( (1 << 7) | (0 << 6) | (0 << 5) | (0 << 4) )	@D3~0
	str r0, [r1]


	@ Go...
	ldr r0, PMSR0_ADDR  @ read a return address
	ldr r1, [r0]
	mov pc, r1
	nop
	nop
	1:  b   1b      @ infinite loop
#endif

