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

#include <s3c2460.h>

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
	ldr	r0, =0xff000fff
	bic	r1, pc, r0		/* r0 <- current base addr of code */
	ldr	r2, _TEXT_BASE		/* r1 <- original base addr in ram */
	bic	r2, r2, r0		/* r0 <- current base addr of code */
	cmp     r1, r2                  /* compare r0, r1                  */
	beq     1f			/* r0 == r1 then skip sdram init   */

	/* init ddr sdram controller */
	bl sdram_bus_asm_init

#if defined (CONFIG_USE_MDDR_SDRAM)
#ifdef CONFIG_PKGTYPE_496
	ldr	r0, =sdram_bank0_set_val
	bl	ddr_ram_asm_init
#endif
	ldr	r0, =sdram_bank1_set_val
	bl	ddr_ram_asm_init
#elif defined (CONFIG_USE_MSDR_SDRAM)
#ifdef CONFIG_PKGTYPE_496
	ldr	r0, =sdram_bank0_set_val
	bl	sdr_ram_asm_init
#endif
	ldr	r0, =sdram_bank1_set_val
	bl	sdr_ram_asm_init
#else
#error not select DRAM Type (board/samsung/smdk2460/lowlevel_init.S)
#endif
1:

	ldr	r0, =ELFIN_UART_BASE
	ldr	r1, =0x4b4b4b4b
	str	r1, [r0, #0x20]

	mov	lr, r12
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

	ldr	r0, =ELFIN_CLOCK_POWER_BASE

	ldr     r1, [r0, #CLKDIVCON_OFFSET]	/*Set Clock Divider*/
        bic	r1, r1, #0x00010000
        bic	r1, r1, #0x00000007
        ldr	r2, =0x00004005
        orr	r1, r1, r2
        str	r1, [r0, #CLKDIVCON_OFFSET]

	ldr	r1, =0x0fff0fff		/*Set MPLL Locktime*/
	str	r1, [r0, #LOCKCON0_OFFSET]

	ldr	r1,=0x002a0100		/*Set MPLL*/
	str	r1, [r0, #MPLLCON_OFFSET]

	mov	r1, #0x10		/*Select MPLL clock out for SYSCLK*/
	str	r1, [r0, #CLKSRCCON_OFFSET]

	/* wait at least 200us to stablize all clock */
	mov	r2, #0x10000
1:	subs	r1, r1, #1
	bne	1b

	mov	pc, lr

/*
 * uart_asm_init: Initialize UART in asm mode, 115200bps fixed.
 * void uart_asm_init(void)
 */
uart_asm_init:
	/* set GPIO to enable UART */
	ldr	r0, =ELFIN_GPIO_BASE
	ldr	r1, =0x0000aaaa
	str	r1, [r0, #GPHCON_OFFSET]    @GPIO

	ldr	r0, =ELFIN_UART_BASE
	mov	r1, #0x0
	str	r1, [r0, #0x8]
	str	r1, [r0, #0xC]

	mov	r1, #0x3                     @was 0.
	str	r1, [r0, #0x0]

	ldr	r1, =0x245
	str	r1, [r0, #0x4]

	ldr	r1, =0x1a
	str	r1, [r0, #0x28]

	ldr	r1, =0x4f4f4f4f
	str	r1, [r0, #0x20]

	mov	pc, lr

/*
 * sdram_bus_asm_init: before sdram init, some settings are necessary.
 * void sdram_bus_asm_init(void)
 */
sdram_bus_asm_init:
	ldr	r0, =ELFIN_BUS_SPE_BASE
#ifdef CONFIG_PKGTYPE_496
	ldr	r1, [r0, #PRIORITY_PORT1_OFFSET]	/* Set Memory port1 configuration */
	bic	r1, r1, #0x00000010
	str	r1, [r0, #PRIORITY_PORT1_OFFSET]
#endif
	ldr	r1, [r0, #PRIORITY_PORT2_OFFSET]	/* Set Memory port2 configuration */
	bic	r1, r1, #0x00000010
	str	r1, [r0, #PRIORITY_PORT2_OFFSET]

	mov	pc, lr

/*
 * Nand Interface Init for smdk2460
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

var_in_lowlevel_init:
	.ltorg
/*
 * struct sdram_val {
 *	ulong bank_base;	// r5, reg base address of bank 0
 *	ulong bankcfg_val;	// r6, val of bankcfg
 *	ulong bankcon_val;	// r7, val of bankcon
 *	ulong refresh_val;	// r8, val of refresh
 *	ulong timeout_val;	// r9, val of write buffer timeout
 * }
 */
 #ifdef CONFIG_PKGTYPE_496
sdram_bank0_set_val:
	.word	ELFIN_MEMCTL0_BASE
	.word	CFG_BANK0_CFG_VAL
	.word	CFG_BANK0_CON_VAL
	.word	CFG_BANK0_REFRESH_VAL
	.word	CFG_BANK0_TIMEOUT_VAL
#endif

sdram_bank1_set_val:
	.word	ELFIN_MEMCTL1_BASE
	.word	CFG_BANK1_CFG_VAL
	.word	CFG_BANK1_CON_VAL
	.word	CFG_BANK1_REFRESH_VAL
	.word	CFG_BANK1_TIMEOUT_VAL

#ifdef CONFIG_ENABLE_MMU

/*
 * MMU Table for SMDK2460
 */

	/* form a first-level section entry */
.macro FL_SECTION_ENTRY base,ap,d,c,b
	.word (\base << 20) | (\ap << 10) | \
	      (\d << 5) | (1<<4) | (\c << 3) | (\b << 2) | (1<<1)
.endm
.section .mmudata, "a"
	.align 14
	// the following alignment creates the mmu table at address 0x4000.
	.globl mmu_table
mmu_table:
	.set __base,0
	// 1:1 mapping for debugging
	.rept 0x600
	FL_SECTION_ENTRY __base,3,0,0,0
	.set __base,__base+1
	.endr

	// access is not allowed.
	.rept 0xC00 - 0x600
	.word 0x00000000
	.endr

#if defined (CONFIG_PKGTYPE_416)
#ifdef CONFIG_USE_MDDR_SDRAM
	// 32MB for SDRAM 0xC0000000 -> 0x20000000
	.set __base, 0x200
	.rept 0xC20 - 0xC00
	FL_SECTION_ENTRY __base,3,0,1,1
	.set __base,__base+1
	.endr

	// 32MB for SDRAM 0xc2000000 -> 0x30000000
	.set __base, 0x300
	.rept 0xc40 - 0xc20
	FL_SECTION_ENTRY __base,3,0,1,1
	.set __base,__base+1
	.endr
	
#else /* CONFIG_USE_MSDR_SDRAM */
	// 64MB for SDRAM 0xC0000000 -> 0x20000000
	.set __base, 0x200
	.rept 0xC40 - 0xC00
	FL_SECTION_ENTRY __base,3,0,1,1
	.set __base,__base+1
	.endr
#endif

#elif defined (CONFIG_PKGTYPE_496)
	// 32MB for SDRAM 0xC0000000 -> 0x10000000
	.set __base, 0x100
	.rept 0xC20 - 0xC00
	FL_SECTION_ENTRY __base,3,0,1,1
	.set __base,__base+1
	.endr

	// 32MB for SDRAM 0xc2000000 -> 0x20000000
	.set __base, 0x200
	.rept 0xc40 - 0xc20
	FL_SECTION_ENTRY __base,3,0,1,1
	.set __base,__base+1
	.endr
#else
#error not select Package Type (board/samsung/smdk2460/lowlevel_init.S)
#endif

	// access is not allowed.
	.rept 0x1000 - 0xc40
	.word 0x00000000
	.endr

#endif

