/*
 * (C) Copyright 2006 DENX Software Engineering
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

#include <common.h>


#ifdef CONFIG_ONENAND
#include <onenand.h>
#include <regs.h>

#include <asm/io.h>

#if 0
static int s3c_scan_bbt(struct mtd_info *mtdinfo)
{
	return 0;
}
#endif

/*
 * Board-specific OneNAND initialization. The following members of the
 * argument are board-specific (per include/linux/mtd/onenand.h):
 * - base : address that OneNAND is located at.
 * - scan_bbt: board specific bad block scan function.
 */
int board_onenand_init (struct onenand_chip *onenand)
{
	volatile uint *base = 0x0;
	uint temp;

//	onenand->scan_bbt = s3c_scan_bbt;
	onenand->base = CFG_ONENAND_BASE;
	temp = *base;
	*base = 0x00900090;
	if (0x00900090 == *base) {
		*base = temp;
		return 0;
	}
	else {
		if ((*base & 0xff00ffff) == 0x000000ec) {
//			onenand->base = 0;
			return 0;
		}
		*base = temp;
	}
	return -1;
}
#endif /* CONFIG_ONENAND */
