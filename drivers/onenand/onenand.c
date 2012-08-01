/*
 * (C) Copyright 2005
 * 2N Telekomunikace, a.s. <www.2n.cz>
 * Ladislav Michl <michl@2n.cz>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
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

#if defined(CONFIG_S3C64XX)

#include <s3c_onenand.h>

#ifndef CFG_ONENAND_BASE_LIST
#define CFG_ONENAND_BASE_LIST { CFG_ONENAND_BASE }
#endif

int onenand_curr_device = -1;
onenand_info_t onenand_info[CFG_MAX_ONENAND_DEVICE];

static struct onenand_chip onenand_chip[CFG_MAX_ONENAND_DEVICE];
static ulong base_address[CFG_MAX_ONENAND_DEVICE] = CFG_ONENAND_BASE_LIST;

static const char default_onenand_name[] = "OneNAND";

extern int board_onenand_init(struct onenand_chip *onenand);

static void onenand_init_chip(struct mtd_info *mtd, struct onenand_chip *onenand,
			   ulong base_addr)
{
	mtd->priv = onenand;

	if (board_onenand_init(onenand))
		return;

	if (onenand_scan(mtd, CFG_MAX_ONENAND_DEVICE) == 0) {
		if (!mtd->name)
			mtd->name = (char *)default_onenand_name;
	} else
		mtd->name = NULL;
}

void onenand_init(void)
{
	int i;
	unsigned int size = 0;
	for (i = 0; i < CFG_MAX_ONENAND_DEVICE; i++) {
		onenand_init_chip(&onenand_info[i], &onenand_chip[i], base_address[i]);
		size += onenand_info[i].size;
		if (onenand_curr_device == -1)
			onenand_curr_device = i;
}
	printf("%lu MB\n", size / (1024 * 1024));
}

#endif

