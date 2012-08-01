/*
 *  armboot - Startup Code for S3C64xx/ARM1176 CPU-core
 *
 *  Copyright (c) 2007	Samsung Electronics
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * 2007-09-21 - Restructured codes by jsgood (jsgood.yang@samsung.com)
 * 2007-09-21 - Added moviNAND and OneNAND boot codes by jsgood (jsgood.yang@samsung.com)
 * Base codes by scsuh (sc.suh)
 */

#include <config.h>
#include <version.h>
#include <regs.h>

_TEXT_PHY_BASE:
	.word	CFG_PHY_UBOOT_BASE

.globl load_bl2_irom

load_bl2_irom:
	ldr	sp, _TEXT_PHY_BASE
	ldr	r0, =ELFIN_GPIO_BASE
	ldr	r1, [r0, #GPNCON_OFFSET]
	bic	r1, r1, #0xfc000000
	str	r1, [r0, #GPNCON_OFFSET]

	ldr	r1, [r0, #GPNPUD_OFFSET]
	bic	r1, r1, #0xfc000000
	str	r1, [r0, #GPNPUD_OFFSET]

	ldr	r1, [r0, #GPNDAT_OFFSET]
	mov	r2, #0x7
	and	r1, r2, r1, lsr #0xd

	ldr	r0, =ELFIN_CLOCK_POWER_BASE
	str	r1, [r0, #INF_REG3_OFFSET]

	cmp	r1, #0x1
	beq	onenand

	bl	nand_asm_init

	cmp	r1, #0x0
	beq	movinand

	cmp	r1, #0x7
	beq	movinand

nand:
	cmp	r3, #0xf
	beq	after_copy

	cmp	r1, #0x4
	beq	nand_adv

	cmp	r1, #0x5
	beq	nand_adv

	bl	nand_bl2_copy
	b	after_copy

nand_adv:
	bl	nand_bl2_copy_adv
	b	after_copy

movinand:
	cmp	r3, #0xf
	beq	after_copy
	bl	movi_bl2_copy
	b	after_copy

onenand:
	cmp	r3, #0xf
	beq	after_copy
	bl	onenand_bl2_copy
	b	after_copy

