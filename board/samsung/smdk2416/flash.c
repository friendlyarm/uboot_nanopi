/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
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

#undef FLASH_DEBUG

#undef CFG_FLASH_BASE
#define CFG_FLASH_BASE 0x08000000

ulong myflush (void);


#define FLASH_BANK_SIZE	PHYS_FLASH_SIZE
#define MAIN_SECT_SIZE  0x10000	/* 64 KB */

flash_info_t flash_info[CFG_MAX_FLASH_BANKS];

#undef  CFG_FLASH_WORD_SIZE
#define CFG_FLASH_WORD_SIZE unsigned short

#define CMD_READ_ARRAY		0x000000F0
#define CMD_UNLOCK1		0x000000AA
#define CMD_UNLOCK2		0x00000055
#define CMD_ERASE_SETUP		0x00000080
#define CMD_ERASE_CONFIRM	0x00000030
#define CMD_PROGRAM		0x000000A0
#define CMD_UNLOCK_BYPASS	0x00000020

#if 0
#define MEM_FLASH_ADDR1		(*(volatile u16 *)(CFG_FLASH_BASE + (0x00000555 << 1)))
#define MEM_FLASH_ADDR2		(*(volatile u16 *)(CFG_FLASH_BASE + (0x000002AA << 1)))
#endif
#define CFG_FLASH_ADDR0		0x555
#define CFG_FLASH_ADDR1		0x2AA
#define BIT_ERASE_DONE		0x00000080
#define BIT_RDY_MASK		0x00000080
#define BIT_PROGRAM_ERROR	0x00000020
#define BIT_TIMEOUT		0x80000000	/* our flag */

#define READY 1
#define ERR   2
#define TMO   4

#define CFG_MONITOR_BASE CFG_FLASH_BASE
#define CFG_MONITOR_LEN (256*1024)

#include "../common/flash_common.c"

uint nor_flash_base = CFG_FLASH_BASE;

/*-----------------------------------------------------------------------
 */

ulong flash_init (void)
{
	unsigned long total_b = 0;
	unsigned long size_b[CFG_MAX_FLASH_BANKS];
	int i;

	{
		vu_long * base_addr = (vu_long*)0x400;
		ulong temp;

		temp = *base_addr;
		*base_addr = 0x20764316;
		if (*base_addr != 0x20764316)
			nor_flash_base = 0;
		*base_addr = temp;
	}

	for (i = 0; i < CFG_MAX_FLASH_BANKS; i++) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
		flash_info[i].sector_count = -1;
		flash_info[i].size = 0;

		/* call flash_get_size() to initialize sector address */
		size_b[i] = flash_get_size((vu_long *)nor_flash_base, &flash_info[i]);
		flash_info[i].size = size_b[i];
		if (flash_info[i].flash_id == FLASH_UNKNOWN) {
#ifdef FLASH_DEBUG
			printf("## Unknown FLASH on Bank %d - Size = 0x%08lx = %ld MB\n",
			       i, size_b[i], size_b[i] << 20);
#endif
			flash_info[i].sector_count = -1;
			flash_info[i].size = 0;
		}


		total_b += flash_info[i].size;
	}

#if 0	/* by scsuh. removed for simple usage */
	/* Monitor protection ON by default */
	(void)flash_protect(FLAG_PROTECT_SET, CFG_MONITOR_BASE,
			    CFG_MONITOR_BASE + CFG_MONITOR_LEN - 1,
			    &flash_info[0]);
#endif

#ifdef CFG_ENV_IS_IN_FLASH
	(void)flash_protect(FLAG_PROTECT_SET, CFG_ENV_ADDR,
			    CFG_ENV_ADDR + CFG_ENV_SECT_SIZE - 1,
			    &flash_info[0]);
	(void)flash_protect(FLAG_PROTECT_SET, CFG_ENV_ADDR_REDUND,
			    CFG_ENV_ADDR_REDUND + CFG_ENV_SECT_SIZE - 1,
			    &flash_info[0]);
#endif

	return total_b;
}

