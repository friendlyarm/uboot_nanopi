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

#include <s3c24a0.h>
#include "smdk24a0_val.h"

#define BWSCON	0x48000000

/* BWSCON */
#define DW8		 	(0x0)
#define DW16		 	(0x1)
#define DW32		 	(0x2)
#define WAIT		 	(0x1<<2)
#define UBLB		 	(0x1<<3)

#define B1_BWSCON	  	(DW32)
#define B2_BWSCON	  	(DW16)
#define B3_BWSCON	  	(DW16 + WAIT + UBLB)
#define B4_BWSCON	  	(DW16)
#define B5_BWSCON	  	(DW16)
#define B6_BWSCON	  	(DW32)
#define B7_BWSCON	  	(DW32)

/* BANK0CON */
#define B0_Tacs		 	0x0	/*  0clk */
#define B0_Tcos		 	0x0	/*  0clk */
#define B0_Tacc		 	0x7	/* 14clk */
#define B0_Tcoh		 	0x0	/*  0clk */
#define B0_Tah		 	0x0	/*  0clk */
#define B0_Tacp		 	0x0
#define B0_PMC		 	0x0	/* normal */

/* BANK1CON */
#define B1_Tacs		 	0x0	/*  0clk */
#define B1_Tcos		 	0x0	/*  0clk */
#define B1_Tacc		 	0x7	/* 14clk */
#define B1_Tcoh		 	0x0	/*  0clk */
#define B1_Tah		 	0x0	/*  0clk */
#define B1_Tacp		 	0x0
#define B1_PMC		 	0x0

#define B2_Tacs		 	0x0
#define B2_Tcos		 	0x0
#define B2_Tacc		 	0x7
#define B2_Tcoh		 	0x0
#define B2_Tah		 	0x0
#define B2_Tacp		 	0x0
#define B2_PMC		 	0x0

#define B3_Tacs		 	0x0	/*  0clk */
#define B3_Tcos		 	0x3	/*  4clk */
#define B3_Tacc		 	0x7	/* 14clk */
#define B3_Tcoh		 	0x1	/*  1clk */
#define B3_Tah		 	0x0	/*  0clk */
#define B3_Tacp		 	0x3     /*  6clk */
#define B3_PMC		 	0x0	/* normal */

#define B4_Tacs		 	0x0	/*  0clk */
#define B4_Tcos		 	0x0	/*  0clk */
#define B4_Tacc		 	0x7	/* 14clk */
#define B4_Tcoh		 	0x0	/*  0clk */
#define B4_Tah		 	0x0	/*  0clk */
#define B4_Tacp		 	0x0
#define B4_PMC		 	0x0	/* normal */

#define B5_Tacs		 	0x0	/*  0clk */
#define B5_Tcos		 	0x0	/*  0clk */
#define B5_Tacc		 	0x7	/* 14clk */
#define B5_Tcoh		 	0x0	/*  0clk */
#define B5_Tah		 	0x0	/*  0clk */
#define B5_Tacp		 	0x0
#define B5_PMC		 	0x0	/* normal */

#define B6_MT		 	0x3	/* SDRAM */
#define B6_Trcd	 	 	0x1
#define B6_SCAN		 	0x1	/* 9bit */

#define B7_MT		 	0x3	/* SDRAM */
#define B7_Trcd		 	0x1	/* 3clk */
#define B7_SCAN		 	0x1	/* 9bit */

/* REFRESH parameter */
#define REFEN		 	0x1	/* Refresh enable */
#define TREFMD		 	0x0	/* CBR(CAS before RAS)/Auto refresh */
#define Trp		 	0x0	/* 2clk */
#define Trc		 	0x3	/* 7clk */
#define Tchr		 	0x2	/* 3clk */
#define REFCNT		 	1113	/* period=15.6us, HCLK=60Mhz, (2048+1-15.6*60) */
/**************************************/

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
	ldr	r0, =0xfff
	bic	r0, pc, r0		/* r0 <- current base addr of code */
	ldr	r1, _TEXT_BASE		/* r1 <- original base addr in ram */
	cmp     r0, r1                  /* compare r0, r1                  */
	beq     stack_setup		/* r0 == r1 then skip sdram init   */

	bl	mem_con_init

	mov	lr, r12
	mov	pc, lr

/*
 * CPU init setting including watchdog, irq, clock.
 * very important initialization code with care.
 */
system_clock_init:
	/* Disable Watchdog */
	ldr	r0, =ELFIN_WATCHDOG_BASE
	mov	r1, #0x0
	str	r1, [r0, #WTCON_OFFSET]

	/* Disable all interrupts */
	ldr	r0, =ELFIN_INTERRUPT_BASE
	mvn	r1, #0x0
	str	r1, [r0, #INTMSK_OFFSET]

	/* Disable all sub-interrupts */
	str	r1, [r0, #INTSUBMSK_OFFSET]

	/* Set all interrupts as IRQ */
	mov	r1, #0x0
	str	r1, [r0, #INTMOD_OFFSET]

	/* Disable LCD sub interrupts */
	ldr	r0, =ELFIN_LCD_BASE
	mov	r1, #0x3
	str	r1, [r0, #LCDBGCON_OFFSET]


	/* InitClock: Initialize core clock and bus clock. */
	ldr	r0, =ELFIN_CLOCK_POWER_BASE

	ldr	r1, LOCKTIME_val
	str	r1, [r0, #LOCKTIME_OFFSET]

	ldr	r1, CLKDIV_val
	str	r1, [r0, #CLKDIVN_OFFSET]

	ldr	r1, MPLLCON_val
	str	r1, [r0, #MPLLCON_OFFSET]
	/* after clock setting we must wait for a while */
	mov	r1, #0x1000
1:	subs	r1, r1, #1
	bne	1b

	mov	pc, lr

/*
 * InitMemCtl: Initialize memory controller
 */

2:
#ifdef CONFIG_PM
	/* Check if this is a wake-up from sleep */
	ldr	r0, ALIVECON_ADDR
	ldr	r1, [r0]
	tst	r1, #(1<<1)
	bne     WakeupStart
#endif

	/* setup IO Port GPCON_U */
	mov	r0, #0x44000000
	orr	r0, r0, #0x00800000
	mov	r1, #0x02A80000
	str	r1, [r0, #0x00]

	/*
	 * simple init for NAND
	 */
nand_asm_init:
	ldr	r0, =ELFIN_NAND_BASE
	ldr	r1, [r0, #NFCONF_OFFSET]
	orr	r1, r1, #0xf0
	orr	r1, r1, #0xff00
	str     r1, [r0]

	ldr	r1, [r0, #NFCONT_OFFSET]
	orr	r1, r1, #0x03
	str     r1, [r0, #NFCONT_OFFSET]

	mov	pc, lr

uart_asm_init:

	mov	pc, lr

#ifdef CONFIG_PM
WakeupStart:
	/* Go ... */
	ldr r0, GPRAM0_ADDR
	ldr r1, [r0]
	mov pc, r1
	nop
	nop

	/* never occur... */
1:	b   1b
#endif

lowlevel_init_val:
	.ltorg
#ifdef CONFIG_PM
ALIVECON_ADDR:		.long	0x44800044
GPRAM0_ADDR:		.long	0x44800080
#endif

/* important setting values */
CLKDIV_val:		.long	CLKDIV_VAL
MPLLCON_val:		.long	MPLL_VAL
LOCKTIME_val:		.long	0x0fff0fff
gpcon_u_val:		.long	GPCON_U_VAL
gpcon_m_val:		.long	GPCON_M_VAL
gpcon_l_val:		.long	GPCON_L_VAL
gpup_val:		.long	GPUP_VAL

