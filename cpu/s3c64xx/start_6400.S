/*
 *  armboot - Startup Code for S3C6400/ARM1176 CPU-core
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

#ifdef CONFIG_BOOT_ONENAND

.globl load_bl1_6400_onenand

load_bl1_6400_onenand:
	ldr	r0, =0x70000000		@ onenand controller setup
	orr	r0, r0, #0x100000
	ldr	r1, =0x4000
	orr	r1, r1, #0xe0
	str	r1, [r0]

	mov	r1, =0x20000000		@ start buffer register
	orr	r1, r1, #0xc30000
	orr	r1, r1, #0xc800

	sub	r0, r1, #0x0400		@ start address1 register

	ldr	r2, [r1, #0x84]		@ ecc bypass
	orr	r2, r2, #0x100
	str	r2, [r1, #0x84]

	mov	r3, #0x0		@ DFS, FBA
	str	r3, [r0, #0x00]
	str	r3, [r0, #0x04]		@ select dataram for DDP as 0

	mov	r4, #0x104		@ interrupt register
	mov	r5, #0x0002		@ FPA, FSA
	mov	r6, #0x0800		@ BSA

onenand_bl1_load:
	str	r5, [r0, #0x1c]		@ save FPA, FSA
	orr	r6, r6, #0x02		@ BSC
	str	r6, [r1, #0x00]		@ save BSA, BSC
	str	r3, [r1, r4]		@ clear interrupt
	str	r3, [r1, #0x80]		@ write load command

	mov	r7, #0x100		@ need small delay

onenand_wait_loop1:
	subs	r7, r7, #0x1
	bne	onenand_wait_loop1

	add	r5, r5, #0x2		@ next FPA, FSA
	sub	r6, r6, #0x2
	add	r6, r6, #0x200		@ next BSA
	cmp	r5, #0x8
	bne	onenand_bl1_load
	mov	pc, lr
#endif

.globl load_bl2_6400

load_bl2_6400:
#ifdef CONFIG_BOOT_NOR			/* relocate U-Boot to RAM */
	adr	r0, _start		/* r0 <- current position of code   */
	ldr	r1, _TEXT_PHY_BASE	/* r1 <- destination                */
	ldr	r2, _armboot_start
	ldr	r3, _bss_start
	sub	r2, r3, r2		/* r2 <- size of armboot            */
	add	r2, r0, r2		/* r2 <- source end address         */

nor_copy_loop:
	ldmia	r0!, {r3-r10}		/* copy from source address [r0]    */
	stmia	r1!, {r3-r10}		/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end addreee [r2]    */
	ble	nor_copy_loop
	b	after_copy
#endif

#ifdef CONFIG_BOOT_NAND
	mov	r0, #0x1000
	bl	copy_from_nand
	b	after_copy
#endif

#ifdef CONFIG_BOOT_MOVINAND
	ldr	sp, _TEXT_PHY_BASE
	bl	movi_bl2_copy
	b	after_copy
#endif

#ifdef CONFIG_BOOT_ONENAND
	ldr	sp, =0x50000000		@ temporary stack

	mov	r1, =0x20000000		@ start buffer register
	orr	r1, r1, #0xc30000
	orr	r1, r1, #0xc800

	ldr	r2, [r1, #0x84]		@ ecc bypass
	orr	r2, r2, #0x100
	str	r2, [r1, #0x84]

	sub	r0, r1, #0x0400		@ start address1 register

	str	r3, [r0, #0x00]
	str	r3, [r0, #0x04]		@ select dataram for DDP as 0

	mov	r4, #0x104		@ interrupt register

	mov	r6, #0x0c00		@ fixed dataram1 sector number
	str	r6, [r1, #0x00]

	mov	r3, #0x0		@ DFS, FBA
	mov	r5, #0x0000		@ FPA, FSA
	ldr	r9, =CFG_PHY_UBOOT_BASE	@ destination

onenand_bl2_load:
	str	r3, [r0, #0x00]		@ save DFS, FBA
	str	r5, [r0, #0x1c]		@ save FPA, FSA

	mov	r7, #0x0		@ clear interrupt
	str	r7, [r1, r4]
	str	r7, [r1, #0x80]		@ write load command

	mov	r8, #0x1000
onenand_wait_loop2:
	subs	r8, r8, #0x1
	bne	onenand_wait_loop2

onenand_wait_int:			@ wait INT and RI
	ldr	r7, [r1, r4]
	mov	r8, #0x8000
	orr	r8, r8, #0x80
	tst	r7, r8
	beq	onenand_wait_int

	mov	r7, #0x0		@ clear interrupt
	str	r7, [r1, r4]

	mov	r8, #0xc00		@ source address (dataram1)
	mov	r10, #0x40		@ copy loop count (64 = 2048 / 32)

	stmia	sp, {r0-r7}		@ backup

onenand_copy_to_ram:
	ldmia	r8!, {r0-r7}
	stmia	r9!, {r0-r7}
	subs	r10, r10, #0x1
	bne	onenand_copy_to_ram

	ldmia	sp, {r0-r7}		@ restore

	add	r5, r5, #0x4		@ next FPA
	cmp	r5, #0x100		@ last FPA?
	bne	onenand_bl2_load

	/* next block */
	mov	r5, #0x0		@ reset FPA
	add	r3, r3, #0x1		@ next FBA
	cmp	r3, #0x2		@ last FBA?
	bne	onenand_bl2_load
	b	after_copy
#endif

