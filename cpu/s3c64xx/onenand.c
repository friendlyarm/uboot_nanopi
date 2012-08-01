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
#include <s3c_onenand.h>
#include <regs.h>

#include <asm/io.h>

static int s3c_scan_bbt(struct mtd_info *mtdinfo)
{
	return 0;
}

/*
 * Setting address width registers
 * (FBA_WIDTH, FPA_WIDTH, FSA_WIDTH, DFS_DBS_WIDTH)
 */
static void set_addr_width_regs(struct onenand_chip *onenand)
{
	int dev_id, ddp, density;
	int w_dfs_dbs = 0, w_fba = 10, w_fpa = 6, w_fsa = 2;

	dev_id = readl(onenand->base + ONENAND_REG_DEVICE_ID);

	ddp = dev_id & ONENAND_DEVICE_IS_DDP;
	density = dev_id >> ONENAND_DEVICE_DENSITY_SHIFT;

	switch (density & 0xf) {
	case ONENAND_DEVICE_DENSITY_128Mb:
		w_dfs_dbs = 0;
		w_fba = 8;
		w_fpa = 6;
		w_fsa = 1;
		break;

	case ONENAND_DEVICE_DENSITY_256Mb:
		w_dfs_dbs = 0;
		w_fba = 9;
		w_fpa = 6;
		w_fsa = 1;
		break;

	case ONENAND_DEVICE_DENSITY_512Mb:
		w_dfs_dbs = 0;
		w_fba = 9;
		w_fpa = 6;
		w_fsa = 2;
		break;

	case ONENAND_DEVICE_DENSITY_1Gb:
		if (ddp) {
			w_dfs_dbs = 1;
			w_fba = 9;
		} else {
			w_dfs_dbs = 0;
			w_fba = 10;
		}

		w_fpa = 6;
		w_fsa = 2;
		break;

	case ONENAND_DEVICE_DENSITY_2Gb:
		if (ddp) {
			w_dfs_dbs = 1;
			w_fba = 10;
		} else {
			w_dfs_dbs = 0;
			w_fba = 11;
		}

		w_fpa = 6;
		w_fsa = 2;
		break;

	case ONENAND_DEVICE_DENSITY_4Gb:
		if (ddp) {
			w_dfs_dbs = 1;
			w_fba = 11;
		} else {
			w_dfs_dbs = 0;
			w_fba = 12;
		}

		w_fpa = 6;
		w_fsa = 2;
		break;
	}

	writel(w_fba, onenand->base + ONENAND_REG_FBA_WIDTH);
	writel(w_fpa, onenand->base + ONENAND_REG_FPA_WIDTH);
	writel(w_fsa, onenand->base + ONENAND_REG_FSA_WIDTH);
	writel(w_dfs_dbs, onenand->base + ONENAND_REG_DBS_DFS_WIDTH);
}

/*
 * Board-specific NAND initialization. The following members of the
 * argument are board-specific (per include/linux/mtd/nand.h):
 * - base : address that OneNAND is located at.
 * - scan_bbt: board specific bad block scan function.
 * Members with a "?" were not set in the merged testing-NAND branch,
 * so they are not set here either.
 */
int board_onenand_init (struct onenand_chip *onenand)
{
	int value;

	onenand->base = (void __iomem *)CFG_ONENAND_BASE;
	onenand->options |= (ONENAND_READ_BURST | ONENAND_CHECK_BAD | ONENAND_PIPELINE_AHEAD);
	onenand->scan_bbt = s3c_scan_bbt;

	/*** Initialize Controller ***/

	/* SYSCON */
	value = readl(ELFIN_CLOCK_POWER_BASE + CLK_DIV0_OFFSET);
	value = (value & ~(3 << 16)) | (1 << 16);
	writel(value, ELFIN_CLOCK_POWER_BASE + CLK_DIV0_OFFSET);

#if defined(CONFIG_S3C6410) || defined(CONFIG_S3C6430)
	writel(ONENAND_FLASH_AUX_WD_DISABLE, ONENAND_REG_FLASH_AUX_CNTRL);
#endif

	/* Cold Reset */
	writel(ONENAND_MEM_RESET_COLD, onenand->base + ONENAND_REG_MEM_RESET);

	/* Access Clock Register */
	writel(ONENAND_ACC_CLOCK_134_67, onenand->base + ONENAND_REG_ACC_CLOCK);

	/* FBA, FPA, FSA, DBS_DFS Width Register */
	set_addr_width_regs(onenand);

	/* Enable Interrupts */
	writel(0x3ff, onenand->base + ONENAND_REG_INT_ERR_MASK);
	writel(ONENAND_INT_PIN_ENABLE, onenand->base + ONENAND_REG_INT_PIN_ENABLE);
	writel(readl(onenand->base + ONENAND_REG_INT_ERR_MASK) & ~(ONENAND_INT_ERR_RDY_ACT), onenand->base + ONENAND_REG_INT_ERR_MASK);

	/* Memory Device Configuration Register */
	value = (ONENAND_MEM_CFG_SYNC_READ | ONENAND_MEM_CFG_BRL_4 | \
			ONENAND_MEM_CFG_BL_16 | ONENAND_MEM_CFG_IOBE | \
			ONENAND_MEM_CFG_INT_HIGH | ONENAND_MEM_CFG_RDY_HIGH);
	writel(value, onenand->base + ONENAND_REG_MEM_CFG);

	/* Burst Length Register */
	writel(ONENAND_BURST_LEN_16, onenand->base + ONENAND_REG_BURST_LEN);

	return 0;
}

