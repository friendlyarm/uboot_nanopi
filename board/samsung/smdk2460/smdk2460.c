/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
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

#include <common.h>
#include <regs.h>

/* ------------------------------------------------------------------------- */

#define M_MDIV	0x03c
#define M_PDIV	0x02
#define M_SDIV	0x0

#define U_M_MDIV	0x030
#define U_M_PDIV	0x2
#define U_M_SDIV	0x1

static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n" "bne 1b":"=r" (loops):"0"(loops));
}

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	SROM_BW_REG &= ~0x1c0;
	SROM_BW_REG |= 0x1c0;
	SROM_BC2_REG = 0x25a0;

	gd->bd->bi_arch_number = MACH_TYPE;
	gd->bd->bi_boot_params = 0x10000100;

#if 0
	icache_enable();
	dcache_enable();
#endif

	return 0;
}

int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

#if defined (CONFIG_PKGTYPE_416) && defined (CONFIG_USE_MSDR_SDRAM)
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
#else /* CONFIG_PKGTYPE_496 or (416 and mDDR) */
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
#endif

	return 0;
}

#ifdef BOARD_LATE_INIT
int board_late_init (void)
{
	uint *magic = (uint*)(PHYS_SDRAM_1);
	char boot_cmd[30];

	if ((0x24564236 == magic[0]) && (0x20764316 == magic[1])) {
		sprintf(boot_cmd, "nandw 0 0x38000 0x%08x", PHYS_SDRAM_1+0x8000);
		magic[0] = 0;
		magic[1] = 0;
		printf("\n\nready for self-burning U-Boot image\n\n");
		setenv("bootdelay", "0");
		setenv("bootcmd", boot_cmd);
	}
}
#endif

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("Board: SMDK2460\n");
	return (0);
}
#endif

#ifdef CONFIG_ENABLE_MMU
ulong virt_to_phy_smdk2460(ulong addr)
{

#ifdef CONFIG_PKGTYPE_416
#ifdef CONFIG_USE_MDDR_SDRAM
	if ((0xc0000000 <= addr) && (addr < 0xc2000000))
		return (addr - 0xc0000000 + 0x20000000);
	else if ((0xc2000000 <= addr) && (addr < 0xc4000000))
		return (addr - 0xc2000000 + 0x30000000);
	else
		printf("do not support this address : %08lx\n", addr);
	
#else /* CONFIG_USE_MSDR_SDRAM */
	if ((0xc0000000 <= addr) && (addr < 0xc4000000))
		return (addr - 0xc0000000 + 0x20000000);
	else
		printf("do not support this address : %08lx\n", addr);
#endif

#else /* CONFIG_PKGTYPE_496 */
	if ((0xc0000000 <= addr) && (addr < 0xc2000000))
		return (addr - 0xc0000000 + 0x10000000);
	else if ((0xc2000000 <= addr) && (addr < 0xc4000000))
		return (addr - 0xc2000000 + 0x20000000);
	else
		printf("do not support this address : %08lx\n", addr);
#endif

	return addr;
}
#endif

#if (CONFIG_COMMANDS & CFG_CMD_NAND) && defined(CFG_NAND_LEGACY)
#include <linux/mtd/nand.h>
extern struct nand_chip nand_dev_desc[CFG_MAX_NAND_DEVICE];
void nand_init(void)
{
	nand_probe(CFG_NAND_BASE);
	if (nand_dev_desc[0].ChipID != NAND_ChipID_UNKNOWN) {
		print_size(nand_dev_desc[0].totlen, "\n");
	}
}
#endif
