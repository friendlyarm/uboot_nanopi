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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _ONENAND_H_
#define _ONENAND_H_

#include <linux/mtd/compat.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>

typedef struct mtd_info onenand_info_t;

extern int onenand_curr_device;
extern onenand_info_t onenand_info[];

static inline int onenand_read(onenand_info_t *info, ulong ofs, ulong *len, u_char *buf)
{
	return info->read(info, ofs, *len, (size_t *)len, buf);
}

static inline int onenand_write(onenand_info_t *info, ulong ofs, ulong *len, u_char *buf)
{
	return info->write(info, ofs, *len, (size_t *)len, buf);
}

#if 0
static inline int onenand_write_ecc(onenand_info_t *info, ulong ofs, ulong *len, u_char *buf,u_char *oobbuf, struct nand_oobinfo *oobsel)
{
        return info->write_ecc(info, ofs, *len, (size_t *)len, buf,oobbuf,NULL);
}
#endif

static inline int onenand_block_isbad(onenand_info_t *info, ulong ofs)
{
	return info->block_isbad(info, ofs);
}

static inline int onenand_erase(onenand_info_t *info, ulong off, ulong size)
{
	struct erase_info instr;

	instr.mtd = info;
	instr.addr = off;
	instr.len = size;
	instr.callback = 0;

	return info->erase(info, &instr);
}

extern int onenand_unlock(struct mtd_info *mtd, loff_t ofs, size_t len);
extern int onenand_lock(struct mtd_info *mtd, loff_t ofs, size_t len);
#endif
