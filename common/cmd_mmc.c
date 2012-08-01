/*
 * (C) Copyright 2003
 * Kyle Harris, kharris@nexus-tech.net
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
#include <command.h>

#if (CONFIG_COMMANDS & CFG_CMD_MMC)

#include <mmc.h>

int do_mmc (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (mmc_init (1) != 0) {
		printf ("No MMC card found\n");
		return 1;
	}
	return 0;
}

/*
int do_testmmc(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	uint start_blk, blknum, tag;
	start_blk = simple_strtoul(argv[1], NULL, 10);
	blknum = simple_strtoul(argv[2], NULL, 10);
	tag = simple_strtoul(argv[3], NULL, 10);

	if(argc < 4)
		return 0;
	
	printf("COMMND --> start_blk = 0x%x, blknum = %d, flag = %d \n", start_blk, blknum, tag);	
	test_mmc(start_blk, blknum, tag);
	return 1;
}

U_BOOT_CMD(
	testmmc,	4,	0,	do_testmmc,
	"testmmc - mmc testing write/read\n",
	NULL
);
*/

U_BOOT_CMD(
	mmcinit,	1,	0,	do_mmc,
	"mmcinit - initialize mmc card\n",
	NULL
);

#endif	/* CFG_CMD_MMC */
