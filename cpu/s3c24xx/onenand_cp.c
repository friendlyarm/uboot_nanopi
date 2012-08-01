/*
 * $Id: onenand_cp.c,v 1.4 2008/11/03 07:59:47 jsgood Exp $
 *
 * (C) Copyright 2006 Samsung Electronics
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

/*
 * You must make sure that all functions in this file are designed
 * to load only U-Boot image.
 *
 * So, DO NOT USE in common read.
 *
 * By scsuh.
 */

#include <common.h>

#ifdef CONFIG_ONENAND

#include <linux/mtd/compat.h>
#include <linux/mtd/mtd.h>
//#include <linux/mtd/bbm.h>
#include <linux/mtd/onenand.h>

#include <asm/io.h>
#include <asm/errno.h>

#define ONENAND_DATARAM0_BASE	(0x400)

#define rONENAND_START_ADDR1	(*(vu_short *)(0x0001E200))
#define rONENAND_START_ADDR2	(*(vu_short *)(0x0001E202))
#define rONENAND_START_ADDR8	(*(vu_short *)(0x0001E20E))

#define rONENAND_START_BUFFER	(*(vu_short *)(0x0001E400))
#define rONENAND_COMMAND	(*(vu_short *)(0x0001E440))
#define rONENAND_CTRLR_STATUS	(*(vu_short *)(0x0001E480))
#define rONENAND_INT_STATUS	(*(vu_short *)(0x0001E482))

#define ONENAND_DATARAM0_0	0x8
#define ONENAND_DATARAM0_1	0x9
#define ONENAND_DATARAM0_2	0xa
#define ONENAND_DATARAM0_3	0xb

#define ONENAND_LOAD_MAINPAGE	0x0000

#define ONENAND_BLOCK_ALIGN	0x20000	/* 128KB */
#define ONENAND_PAGE_ALIGN	0x800	/* 2KB */
#define ONENAND_SECTOR_ALIGN	0x200	/* 512B */

/*
 * You must make sure that
 * this function assumes that all input values are correct.
 * and only block 0, 1 will be read via this function.
 * You also make sure that functions are designed for only
 * minimal parts of OneNAND products. So, when you meet bugs
 * you may modify them.
 *
 * Above assumptions are due to size limitation. by scsuh
 */
static int onenand_load_page(u32 block, u32 page_sector, u32 bsc)
{
	rONENAND_START_ADDR1 = block;
	rONENAND_START_ADDR2 = 0;
	rONENAND_START_ADDR8 = page_sector;
	rONENAND_START_BUFFER = (ONENAND_DATARAM0_0<<8) | bsc;

	rONENAND_INT_STATUS = 0;
	rONENAND_COMMAND = ONENAND_LOAD_MAINPAGE;

	while(!(rONENAND_INT_STATUS & (1<<15)));

	if(rONENAND_CTRLR_STATUS & (1<<10)) {
		return 1;
	}
	return 0;
}

int onenand_copy_to_ram(u32 block, u32 page_sector, u8 *buffer, uint size)
{
	u32 bsc = (size >> 9);
	u32 *src = (u32*)ONENAND_DATARAM0_BASE, *dst = (u32*)buffer;
	u32 i;

	if(onenand_load_page(block, page_sector, bsc) == 0) {
		for (i=0; i<size; i+=4) {
			*dst++ = *src++;
		}
		return size;
	}
	return 1;
}

int onenand_cp(void)
{
	uint off, want_size, read_size;
	uint block, page_sector;
	u8 * buffer = (u8*)(CFG_PHY_UBOOT_BASE+0xc00);

	for (off = 0xc00; off < 0x40000; ) {
		page_sector = (off>>9) & 0xff;
		block = (off>>17);
		if (off & (ONENAND_PAGE_ALIGN-1))
			want_size = off&(ONENAND_PAGE_ALIGN-1);
		else
			want_size = ONENAND_PAGE_ALIGN;

		read_size = onenand_copy_to_ram(block, page_sector, buffer, want_size);
		if (want_size != read_size) {
			return 1;
		}
		buffer += read_size;
		off += read_size;
	}
	return 0;
}

#endif /* CONFIG_ONENAND */

