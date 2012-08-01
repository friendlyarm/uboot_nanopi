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

	gd->bd->bi_arch_number = MACH_TYPE;
	gd->bd->bi_boot_params = 0x30000100;

#if 0
	icache_enable();
	dcache_enable();
#endif

#if defined (CONFIG_SMDK6400)
	SMBWSTRDR5 = 10;                //Bank5 Read Wait State cont. = 10 clk 		Tacc?
	SMBWSTWRR5 = 10;                //Bank5 Write Wait State ctrl.
	SMBWSTOENR5 = 2;                //Bank5 Output Enable Assertion Delay ctrl.     Tcho?
	SMBWSTWENR5 = 2;                //Bank5 Write Enable Assertion Delay ctrl.
	
	SMBCR5 |=  (1<<0); 		//SMWAIT active High, Read Byte Lane Enabl      WS1?
	SMBCR5 &= ~(3<<4);          	//Clear Memory Width
	SMBCR5 |=  (1<<4);          	//Memory Width = 16bit
#endif

#if defined (CONFIG_TABLA)
	SMBWSTRDR1 = 10;                //Bank5 Read Wait State cont. = 10 clk 		Tacc?
	SMBWSTWRR1 = 10;                //Bank5 Write Wait State ctrl.
	SMBWSTOENR1 = 2;                //Bank5 Output Enable Assertion Delay ctrl.     Tcho?
	SMBWSTWENR1 = 2;                //Bank5 Write Enable Assertion Delay ctrl.
	
	SMBCR1 |=  (1<<0); 		//SMWAIT active High, Read Byte Lane Enabl      WS1?
	SMBCR1 &= ~(3<<4);          	//Clear Memory Width
	SMBCR1 |=  (1<<4);          	//Memory Width = 16bit

#endif

	return 0;
}

int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("Board: TABLA\n");
	return (0);
}
#endif

#ifdef CONFIG_ENABLE_MMU
ulong virt_to_phy_tabla(ulong addr)
{
	if ((0xc0000000 <= addr) && (addr < 0xc2000000))
		return (addr - 0xc0000000 + 0x10000000);
	else if ((0xc2000000 <= addr) && (addr < 0xc4000000))
		return (addr - 0xc2000000 + 0x20000000);
	else
		printf("do not support this address : %08lx\n", addr);

	return addr;
}
#endif

#ifdef BOARD_LATE_INIT
int board_late_init (void)
{
/* CPU core voltage control for enhancing board stability*/
/* JaeCheol Lee (2006, 12 ,05) */
#ifdef CFG_I2C_PMIC
	
	unsigned char val1 = 0x51;	// Allowing to change Bucks' Voltage.
	unsigned char val2 = B_1300mV;
	
	puts ("Support PMIC chip.\n");

	i2c_init(CFG_I2C_SPEED, CFG_I2C_SLAVE);

	if(i2c_probe(PMIC_ID)==0)
	{
		puts ("PMIC was founded\n");
	}
	
	if(i2c_write(PMIC_ID, Bucks_DVM, 1, &val1, 1) != 0) {
			puts ("ErrOr writing the chip.\n");
		}
	
	if(i2c_write(PMIC_ID, Buck1_T1, 1, &val2, 1) != 0) {
			puts ("Error writing the chip.\n");
		}	
	switch(val2)
	{
			case B_1100mV:
					puts ("ARM core 1.10v.\n");
					break;
			case B_1150mV:
					puts ("ARM core 1.15v.\n");
					break;
			case B_1200mV:
					puts ("ARM core 1.20v.\n");
					break;
			case B_1250mV:
					puts ("ARM core 1.25v.\n");
					break;
			case B_1300mV:
					puts ("ARM core 1.30v.\n");
					break;
			case B_1350mV:
					puts ("ARM core 1.35v.\n");
					break;
	}
		
#endif
}
#endif

#if (CONFIG_COMMANDS & CFG_CMD_NAND) && defined(CFG_NAND_LEGACY)
#include <linux/mtd/nand.h>
extern struct nand_chip nand_dev_desc[CFG_MAX_NAND_DEVICE];
void nand_init(void)
{
	s3c_onenand_init();
}
#endif
