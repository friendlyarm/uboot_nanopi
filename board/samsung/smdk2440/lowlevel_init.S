/*
 * Memory Setup stuff - taken from blob memsetup.S
 *
 * Copyright (C) 1999 2000 2001 Erik Mouw (J.A.K.Mouw@its.tudelft.nl) and
 *                     Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
 *
 * Modified for the Samsung SMDK2410 by
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#include <config.h>
#include <version.h>

#include <s3c2440.h>
#include "smdk2440_val.h"

_TEXT_BASE:
	.word	TEXT_BASE

	.globl lowlevel_init
lowlevel_init:
	mov	r12, lr

	/* init system clock */
	bl system_clock_init

	/* for UART */
	bl uart_asm_init

	/* simple init for NAND */
	bl nand_asm_init

	/* when we already run in ram, we don't need to relocate U-Boot.
	 * and actually, memory controller must be configured before U-Boot
	 * is running in ram.
	 */
	ldr	r0, =0xf0000fff
	bic	r1, pc, r0	/* r0 <- current base addr of code */
	ldr	r2, _TEXT_BASE	/* r1 <- original base addr in ram */
	bic	r2, r2, r0	/* r0 <- current base addr of code */
	cmp     r1, r2		/* compare r0, r1                  */
	beq     1f		/* r0 == r1 then skip sdram init   */

	adrl    r0, mem_cfg_val
	bl	mem_con_init

	ldr	r0, =ELFIN_UART_BASE
	ldr	r1, =0x4b4b4b4b
	str	r1, [r0, #0x20]

1:	mov	lr, r12
	mov	pc, lr

/*
 * system_clock_init: Initialize core clock and bus clock.
 * void system_clock_init(void)
 */
system_clock_init:
	/* Disable Watchdog */
	ldr	r0, =ELFIN_WATCHDOG_BASE
	mov	r1, #0
	str	r1, [r0]

	/* mask all IRQs by setting all bits in the INTMR - default */
	ldr	r0, =ELFIN_INTERRUPT_BASE
	mov	r1, #0xffffffff
	str	r1, [r0, #INTMSK_OFFSET]
	ldr	r1, =0x000007ff
	str	r1, [r0, #INTSUBMSK_OFFSET]

	/* Initialize System Clock */
	ldr	r0, =ELFIN_CLOCK_POWER_BASE
        ldr	r1, =0x00ffffff
        str	r1, [r0, #LOCKTIME_OFFSET]	/*Set Clock Divider*/
#ifndef S3C2440_PLL_OFF
        /* FCLK:HCLK:PCLK */
	mov	r1, #0x00000000
	str	r1, [r0, #CAMDIVN_OFFSET]
        ldr     r1, =CLKDIVN_VAL
        str     r1, [r0, #CLKDIVN_OFFSET]
#endif
	/* if HDIVN is not ZERO, the CPU bus mode has to be changed from the fast
	 * bus mode to the async bus mode.
	 */
        mrc     p15, 0, r1, c1, c0, 0           @ read ctrl register
        orr     r1, r1, #0xc0000000             @ Asynchronous
        mcr     p15, 0, r1, c1, c0, 0           @ write ctrl register

#ifndef S3C2440_PLL_OFF
	/* UPLL setup */
        ldr	r1, =UPLL_VAL
        str     r1, [r0, #UPLLCON_OFFSET]
#endif
	nop
	nop
	nop
	nop
	nop		@ wait until upll has the effect
	nop
	nop
	nop

#ifndef S3C2440_PLL_OFF
        /* PLL setup */
	ldr	r1, =MPLL_VAL
	str     r1, [r0, #MPLLCON_OFFSET]
#endif

	/* wait at least 200us to stablize all clock */
	mov	r2, #0x10000
1:	subs	r1, r1, #1
	bne	1b

	mov	pc, lr

/*
 * uart_basic_init: Initialize UART in asm mode, 115200bps fixed.
 * void uart_asm_init(void)
 */
uart_asm_init:
	/* set GPIO to enable UART */
	ldr	r0, =ELFIN_GPIO_BASE
	ldr	r1, =0x0000aaaa
	str	r1, [r0, #GPHCON_OFFSET]

	ldr	r0, =ELFIN_UART_BASE
	mov	r1, #0x0
	str	r1, [r0, #0x8]
	str	r1, [r0, #0xC]

	mov	r1, #0x3
	str	r1, [r0, #0x0]

	ldr	r1, =0x245
	str	r1, [r0, #0x4]

#if (CONFIG_SYS_CLK_FREQ == 16934400)

#if defined (CONFIG_SMDK2440)
	ldr	r1, =0x23
#elif defined (CONFIG_SMDK2442)
	ldr	r1, =0x19
#endif
#endif
	str	r1, [r0, #UBRDIV0_OFFSET]

	ldr	r1, =0x4f4f4f4f
	str	r1, [r0, #0x20]

	mov	pc, lr


/*
 * Nand Interface Init for smdk2440
 */
nand_asm_init:
	ldr	r0, =ELFIN_NAND_BASE
	ldr	r1, [r0, #NFCONF_OFFSET]
	orr	r1, r1, #0xf0
	orr	r1, r1, #0xff00
	str     r1, [r0]
#if 0
	ldr	r1, [r0, #NFCONT_OFFSET]
	orr	r1, r1, #0x01
#else
	mov	r1, #0x03
#endif
	str     r1, [r0, #NFCONT_OFFSET]

	mov	pc, lr

	.align 4
mem_cfg_val:
	.long   vBWSCON		/* 0x00 */
	.long   vBANKCON0	/* 0x04 */
	.long   vBANKCON1	/* 0x08 */
	.long   vBANKCON2	/* 0x0c */
	.long   vBANKCON3	/* 0x10 */
	.long   vBANKCON4	/* 0x14 */
	.long   vBANKCON5	/* 0x18 */
	.long   vBANKCON6	/* 0x1c */
	.long   vBANKCON7	/* 0x20 */
	.long   vREFRESH	/* 0x24 */
	.long   vBANKSIZE	/* 0x28 */
	.long   vMRSRB6		/* 0x2c */
	.long   vMRSRB7		/* 0x30 */

var_in_lowlevel_init:
	.ltorg

#ifdef CONFIG_ENABLE_MMU

/*
 * MMU Table for SMDK2440
 */

/* these macro should match with pci window macros */
#define UNCACHED_SDRAM_START	1
#define UNCACHED_SDRAM_SZ	1

	/* form a first-level section entry */
.macro FL_SECTION_ENTRY base,ap,d,c,b
	.word (\base << 20) | (\ap << 10) | \
	      (\d << 5) | (1<<4) | (\c << 3) | (\b << 2) | (1<<1)
.endm
.section .mmudata, "a"
	.align 14
	/* the following alignment creates the mmu table at address 0x4000. */
	.globl mmu_table
mmu_table:
	.set __base,0
	/* 1:1 mapping for debugging */
	.rept 0x600
	FL_SECTION_ENTRY __base,3,0,0,0
	.set __base,__base+1
	.endr

	/* access is not allowed. */
	.rept 0xC00 - 0x600
	.word 0x00000000
	.endr

	/* 128MB for SDRAM 0xC0000000 -> 0x30000000 */
	.set __base, 0x300
	.rept 0xC80 - 0xC00
	FL_SECTION_ENTRY __base,3,0,1,1
	.set __base,__base+1
	.endr

	/* access is not allowed. */
	.rept 0x1000 - 0xc80
	.word 0x00000000
	.endr

#endif

