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

#include <mdirac3.h>
#include "tabla_val.h"

_TEXT_BASE:
	.word	TEXT_BASE

	.globl lowlevel_init
lowlevel_init:
	mov	r12, lr

	ldr	r0,=0x70000013      	@ Peri. Port Setup
    	mcr	p15,0,r0,c15,c2,4       	@ 256M(0x70000000-0x7fffffff)

	/* Disable Watchdog */
	ldr	r0, =ELFIN_WATCHDOG_BASE	@0x7e005000
	mov	r1, #0
	str	r1, [r0]

	@ External interrupt pending clear
	ldr	r0, =(ELFIN_GPIO_BASE+0x87c)	/*EINTPEND*/
	ldr	r1, [r0]
	str	r1, [r0]

	ldr	r0, =ELFIN_VIC0_BASE_ADDR 		@0x71200000	
	ldr	r1, =ELFIN_VIC1_BASE_ADDR 		@0x71300000	

	@ Disable all interrupts ( VIC0 and VIC1)
	mvn	r3, #0x0
	str	r3, [r1, #oINTMSK]
	mvn	r3, #0x0
	str	r3, [r2, #oINTMSK]
	
	@ Set all interrupts as IRQ
	mov	r3, #0x0
	str	r3, [r1, #oINTMOD]
	str	r3, [r2, #oINTMOD]

	@ Pending Interrupt Clear
	mov	r3, #0x0
	str	r3, [r1, #oVECTADDR]
	str	r3, [r2, #oVECTADDR]


	bl system_clock_init

	bl smc_init

	bl memory_init

	/* for UART */
	bl uart_asm_init

	ldr	r0, =(ELFIN_CLOCK_POWER_BASE+RST_STAT_OFFSET)
	ldr	r1, [r0]
	bic	r1, r1, #0xf7
	cmp	r1, #0x8
	beq	wakeup_reset	

	/* when we already run in ram, we don't need to relocate U-Boot.
	 * and actually, memory controller must be configured before U-Boot
	 * is running in ram.
	 */
	ldr	r0, =0xff000fff
	bic	r1, pc, r0		/* r0 <- current base addr of code */
	ldr	r2, _TEXT_BASE		/* r1 <- original base addr in ram */
	bic	r2, r2, r0		/* r0 <- current base addr of code */
	cmp r1, r2                  /* compare r0, r1                  */
	beq	1f			/* r0 == r1 then skip sdram init   */

1:

	ldr	r0, =ELFIN_UART_BASE
	ldr	r1, =0x4b4b4b4b
	str	r1, [r0, #0x20]

	mov	lr, r12
	mov	pc, lr

wakeup_reset:
	/*LED test*/
	ldr 	r0, =(ELFIN_GPIO_BASE+GPH_OFFSET+0x10)	/*GPHCON*/
	ldr	r1, [r0]
	bic	r1, r1, #0xff00
	orr	r1, r1, #0x5500
	str	r1, [r0]

	ldr	r0, =(ELFIN_GPIO_BASE+GPH_OFFSET)	/*GPHDAT*/
	ldr	r1, [r0]
	bic 	r1, r1, #0xf0
	orr	r1, r1, #0x40
	str	r1, [r0]	
	
	/*Clear wakeup status register*/
	ldr	r0, =(ELFIN_CLOCK_POWER_BASE+WAKEUP_STAT_OFFSET)
	ldr	r1, [r0]
	str	r1, [r0]

	/*Load return address and jump to kernel*/	
	ldr	r0, =(ELFIN_CLOCK_POWER_BASE+INF_REG0_OFFSET)
	ldr	r1, [r0]	/* r1 = physical address of mdirac3_cpu_resume function*/
	mov	pc, r1		/*Jump to kernel (sleep-mdirac3.S)*/
	nop
	nop
/*
 * system_clock_init: Initialize core clock and bus clock.
 * void system_clock_init(void)
 */
system_clock_init:
	ldr	r0, =ELFIN_CLOCK_POWER_BASE	@0x7e00f000

	mov	r1,#0xff00
	orr	r1,r1,#0xff
	str	r1, [r0, #MPLL_LOCK_OFFSET]


	ldr   r1, [r0, #CLK_DIV0_OFFSET]	/*Set Clock Divider*/
	bic	r1,r1,#0x10000
	bic	r1,r1,#0xff00
	bic	r1,r1,#0xff    
	ldr	r2,=((Startup_MCLK_DIV<<16)|(Startup_MCLKx2_DIV<<12)|(Startup_APB_DIV<<8)|(Startup_AXI_DIV<<4)|(Startup_ARM_DIV))	@0x11130
       orr	r1, r1, r2
       str	r1, [r0, #CLK_DIV0_OFFSET]


	ldr	r1,=((Startup_PLL_MVAL<<16)|(Startup_PLL_PVAL<<8)|(Startup_PLL_SVAL))		@0xc80301	
	str	r1, [r0, #MPLL_CON_OFFSET]

	ldr	r1,=PLL_EN_OFFSET			@MPLL Enable
	orr	r2,r0,r1
	ldr	r3,[r2]
	orr	r3,r3,#0x1
	str	r3,[r2]	

	ldr	r1, [r0, #CLK_SRC_OFFSET]
	orr	r1,r1,#0x1	
	str	r1, [r0, #CLK_SRC_OFFSET]

	/* wait at least 200us to stablize all clock */
	mov	r1, #0x10000
1:	subs	r1, r1, #1
	bne	1b

	mov	pc, lr


smc_init:
/*
 * SMC initialize
 * 
 */
	ldr	r0, =ELFIN_SMC0_BASE
	ldr	r1,=0x7
	str	r1,[r0, #0x4]
	str	r1,[r0, #0x8]

	ldr	r1,=0x4
	str	r1,[r0, #0x10]
	
	ldr	r1,=0x303011
	str	r1,[r0, #0x14]

	mov	pc, lr

memory_init:

#if defined (CONFIG_USE_DDR_SDRAM)
/*
 * ddr_ram_asm_init: Initialize DDR SDRAM.
 * void ddr_ram_asm_init(ulong MEM_CTLR_BASE)
 */

	ldr	r0, =ELFIN_DMC0_BASE			@DMC0 base address 0x7e000000 

	ldr	r1, =0x4
	str	r1, [r0, #INDEX_DMC_MEMC_CMD]

	ldr	r1, =DMC_DDR_REFRESH_PRD
	str	r1, [r0, #INDEX_DMC_REFRESH_PRD]

	ldr	r1, =DMC_DDR_CAS_LATENCY
	str	r1, [r0, #INDEX_DMC_CAS_LATENCY]

	ldr	r1, =DMC_DDR_t_DQSS
	str	r1, [r0, #INDEX_DMC_T_DQSS]

	ldr	r1, =DMC_DDR_t_MRD
	str	r1, [r0, #INDEX_DMC_T_MRD]

	ldr	r1, =DMC_DDR_t_RAS
	str	r1, [r0, #INDEX_DMC_T_RAS]

	ldr	r1, =DMC_DDR_t_RC
	str	r1, [r0, #INDEX_DMC_T_RC]

	ldr	r1, =DMC_DDR_t_RCD
	ldr	r2, =DMC_DDR_schedule_RCD
	orr	r1, r1, r2
	str	r1, [r0, #INDEX_DMC_T_RCD]

	ldr	r1, =DMC_DDR_t_RFC
	ldr	r2, =DMC_DDR_schedule_RFC
	orr	r1, r1, r2
	str	r1, [r0, #INDEX_DMC_T_RFC]

	ldr	r1, =DMC_DDR_t_RP
	ldr	r2, =DMC_DDR_schedule_RP
	orr	r1, r1, r2
	str	r1, [r0, #INDEX_DMC_T_RP]

	ldr	r1, =DMC_DDR_t_RRD
	str	r1, [r0, #INDEX_DMC_T_RRD]

	ldr	r1, =DMC_DDR_t_WR
	str	r1, [r0, #INDEX_DMC_T_WR]

	ldr	r1, =DMC_DDR_t_WTR
	str	r1, [r0, #INDEX_DMC_T_WTR]

	ldr	r1, =DMC_DDR_t_XP
	str	r1, [r0, #INDEX_DMC_T_XP]

	ldr	r1, =DMC_DDR_t_XSR
	str	r1, [r0, #INDEX_DMC_T_XSR]

	ldr	r1, =DMC_DDR_t_ESR
	str	r1, [r0, #INDEX_DMC_T_ESR]

	ldr	r1, =DMC0_MEM_CFG
	str	r1, [r0, #INDEX_DMC_MEMORY_CFG]

	ldr	r1, =DMC_DDR_16_CFG		@16bit, DDR
	str	r1, [r0, #INDEX_DMC_USER_CONFIG]

	ldr	r1, =DMC0_CHIP0_CFG
	str	r1, [r0, #INDEX_DMC_CHIP_0_CFG]


	@DMC0 DDR Chip 0 configuration direct command reg
	ldr	r1, =DMC_NOP0
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

	@Precharge All
	ldr	r1, =DMC_PA0
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

	@Auto Refresh	2 time
	ldr	r1, =DMC_AR0
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

	@Mode Reg
	ldr	r1, =DMC_mDDR_MR0
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

	@MRS 
	ldr	r1, =DMC_mDDR_EMR0	
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

	@Enable DMC0
	mov	r1, #0x0
	str	r1, [r0, #INDEX_DMC_MEMC_CMD]

Check_DMC0_READY:
	ldr	r1, [r0, #INDEX_DMC_MEMC_STATUS]
	mov	r2, #0x3
	and	r1, r1, r2
	cmp	r1, #0x1
	bne	Check_DMC0_READY

	nop

	
#elif defined (CONFIG_USE_SDR_SDRAM)
@
@ sdr_ram_asm_init: Initialize memory controller
@

	ldr	r0,=ELFIN_DMC0_BASE		@DMC0 base address

	ldr	r1,=0x4
	str	r1,[r0, #INDEX_DMC_MEMC_CMD]   @Enter the Config State

	ldr	r1,=DMC_SDR_REFRESH_PRD
	str	r1,[r0, #INDEX_DMC_REFRESH_PRD]

	ldr	r1,=DMC_SDR_CAS_LATENCY
	str	r1,[r0, #INDEX_DMC_CAS_LATENCY]

	ldr	r1,=DMC_SDR_t_DQSS
	str	r1,[r0, #INDEX_DMC_T_DQSS]

	ldr	r1, =DMC_SDR_t_MRD
	str	r1, [r0, #INDEX_DMC_T_MRD]

	ldr	r1, =DMC_SDR_t_RAS
	str	r1, [r0, #INDEX_DMC_T_RAS]

	ldr	r1, =DMC_SDR_t_RC
	str	r1, [r0, #INDEX_DMC_T_RC]

	ldr	r1, =DMC_SDR_t_RCD
	ldr	r2, =DMC_SDR_schedule_RCD
	orr	r1, r1, r2
	str	r1, [r0, #INDEX_DMC_T_RCD]

	ldr	r1, =DMC_SDR_t_RFC
	ldr	r2, =DMC_SDR_schedule_RFC
	orr	r1, r1, r2
	str	r1, [r0, #INDEX_DMC_T_RFC]

	ldr	r1, =DMC_SDR_t_RP
	ldr	r2, =DMC_SDR_schedule_RP
	orr	r1, r1, r2
	str	r1, [r0, #INDEX_DMC_T_RP]

	ldr	r1, =DMC_SDR_t_RRD
	str	r1, [r0, #INDEX_DMC_T_RRD]

	ldr	r1, =DMC_SDR_t_WR
	str	r1, [r0, #INDEX_DMC_T_WR]

	ldr	r1, =DMC_SDR_t_WTR
	str	r1, [r0, #INDEX_DMC_T_WTR]

	ldr	r1, =DMC_SDR_t_XP
	str	r1, [r0, #INDEX_DMC_T_XP]

	ldr	r1, =DMC_SDR_t_XSR
	str	r1, [r0, #INDEX_DMC_T_XSR]

	ldr	r1, =DMC_SDR_t_ESR
	str	r1, [r0, #INDEX_DMC_T_ESR]

	ldr	r1, =DMC0_MEM_CFG		@1Chip, ARID[3:0], Burst 4, R/C 
	str	r1, [r0, #INDEX_DMC_MEMORY_CFG]

	ldr	r1, =DMC_SDR_16_CFG		@ 16bit, mSDR
	str	r1, [r0, #INDEX_DMC_USER_CONFIG]

	ldr	r1, =DMC0_CHIP0_CFG 		@BANK0 : 64MB
	str	r1, [r0, #INDEX_DMC_CHIP_0_CFG] 

	@ DMC0 SDR Chip 0 configuration direct command reg
	ldr	r1, =DMC_NOP0
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

	@ Precharge All
	ldr	r1, =DMC_PA0
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

	@ Auto Refresh	2 time
	ldr	r1, =DMC_AR0
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

	@Mode Reg
	ldr	r1, =DMC_SDR_MR0			@ MRS, CAS3, BL 4
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]


	@EMRS 
	ldr	r1, =DMC_mSDR_EMR0			@DS:Full, PASR:Full Array
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

	@Enable DMC0
	mov	r1, #0x0			@DMC Ready
	str	r1, [r0, #INDEX_DMC_MEMC_CMD]

	@Check the DMC State
	ldr	r1, [r0, #INDEX_DMC_MEMC_STATUS]
	mov	r2, #0x3
	and	r1, r1, r2
	cmp	r1, #0x1
	nop

#else
#error not select DRAM TYPE
#endif

	mov	pc, lr


/*
 * uart_asm_init: Initialize UART in asm mode, 115200bps fixed.
 * void uart_asm_init(void)
 */
uart_asm_init:
	/* set GPIO to enable UART */
	@ GPIO setting for UART
	ldr	r0,=(ELFIN_GPIO_BASE|GPF_OFFSET|GPIO_CON_OFFSET)
	mov	r1,#0x88
	str   r1,[r0]
	
	ldr	r0, =ELFIN_UART_BASE		@0x7e004000
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
 * Nand Interface Init for smdk2460
 */
nand_asm_init:

	mov	pc, lr

#ifdef CONFIG_ENABLE_MMU

/*
 * MMU Table for SMDK6400
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
	.rept 0xA00
	FL_SECTION_ENTRY __base,3,0,0,0
	.set __base,__base+1
	.endr

	// access is not allowed.
	.rept 0xC00 - 0xA00
	.word 0x00000000
	.endr

	// 64MB for SDRAM 0xC0000000 -> 0x30000000
	.set __base, 0x300
	.rept 0xC40 - 0xC00
	FL_SECTION_ENTRY __base,3,0,1,1
	.set __base,__base+1
	.endr


	// access is not allowed.
	.rept 0x1000 - 0xc40
	.word 0x00000000
	.endr

#endif

