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
#include <linux/mtd/onenand_regs.h>

#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n" "bne 1b":"=r" (loops):"0"(loops));
}

/*
 * Miscellaneous platform dependent initialisations
 */

static void cs8900_pre_init(void)
{
	BANK_CFG_REG &= ~((1<<5)|(1<<4));	//EBI Setting for SROM
						//BANK4,5 conf. = one NAND/SRAM
	SMBIDCYR5_REG = 0;			//Bank5 Idle cycle ctrl.
	SMBWSTWRR5_REG = 14;			//Bank5 Write Wait State ctrl.
	SMBWSTOENR5_REG = 2;			//Bank5 Output Enable Assertion Delay ctrl.     Tcho?
	SMBWSTWENR5_REG = 2;			//Bank5 Write Enable Assertion Delay ctrl.

	SMBWSTRDR5_REG = 14;			//Bank5 Read Wait State cont. = 14 clk	    Tacc?
	SMBCR5_REG |=  ((1<<2)|(1<<0));		//SMWAIT active High, Read Byte Lane Enabl	    WS1?
	SMBCR5_REG &= ~((3<<20)|(3<<12));	//SMADDRVALID = always High when Read/Write
	SMBCR5_REG &= ~(3<<4);			//Clear Memory Width
	SMBCR5_REG |=  (1<<4);			//Memory Width = 16bit
}

#ifdef CONFIG_ONENAND
static void onenand_pre_init(void)
{
	u16 sysconfig1;
	u16* addr = (u16*)(CFG_ONENAND_BASE+ONENAND_REG_SYS_CFG1);

	GPFDAT_REG = (GPFDAT_REG & ~0x8) | 0x8;

	sysconfig1 = *addr;

	SMBCR1_REG = 0x00303012; // non-burst		// Asynchronous
	*addr = (sysconfig1& ~(0xf<<12))|(0<<15)|(4<<12);

	SMBIDCYR1_REG = 0;
	SMBWSTWRR1_REG = 14;
	SMBWSTOENR1_REG = 2;
	SMBWSTWENR1_REG = 2;
	SMBWSTRDR1_REG = 14;  //14clk
	SMBIDCYR1_REG = 0x0;

	SSMCCR_REG |= ((1<<1)|(1<<0));
}
#endif

/*
 * When NAND is not used as Boot Device
 */
static void nand_pre_init(void)
{
	SMBIDCYR3_REG = 0xf;
	SMBWSTRDR3_REG = 0x1f;
	SMBWSTWRR3_REG = 0x1f;
	SMBWSTOENR3_REG = 0;
	SMBWSTWENR3_REG = 1;
	SMBCR3_REG = 0x00303000;
	SMBSR3_REG = 0;
	SMBWSTBDR3_REG = 0x1f;
}

static void max1718_init(unsigned int voltage)
{
	GPBCON_REG = (GPBCON_REG & ~((3<<20)|(0xf<<14))) | ((1<<20)|(0x5<<14));

	GPBDAT_REG &= ~(1<<7);
	switch(voltage) {
		case 120:
			GPFDAT_REG = (GPFDAT_REG & ~0xf0) | 0xB0;
			break;
		case 125:
			GPFDAT_REG = (GPFDAT_REG & ~0xf0) | 0xA0;
			break;
		case 130:
			GPFDAT_REG = (GPFDAT_REG & ~0xf0) | 0x90;
			break;
		case 135:
			GPFDAT_REG = (GPFDAT_REG & ~0xf0) | 0x80;
			break;
		case 140:
		default:
			GPFDAT_REG = (GPFDAT_REG & ~0xf0) | 0x70;
			break;
	}

	GPBDAT_REG &= ~(1<<8);
	GPBDAT_REG |= (1<<10);
	GPBDAT_REG |= (1<<8);
}

int board_init(void)
{
	max1718_init(VDD_ARM);
	nand_pre_init();
	cs8900_pre_init();
#ifdef CONFIG_ONENAND
	onenand_pre_init();
#endif

	SMBCR4_REG = 0x00000015;
	gd->bd->bi_arch_number = MACH_TYPE;
	gd->bd->bi_boot_params = 0x30000100;

#if 0
	icache_enable();
	dcache_enable();
#endif

	return 0;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
#ifdef CONFIG_SDRAM_16BITS
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE >> 1;
#else
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
#endif

	return 0;
}

#ifdef CONFIG_LCD_2412
#define mdelay(x)	udelay(1000*x)
/**************************************************
 Code to initialize the LCD for 2413 SMDK Eval board
 through SPI controlled by GPIO's.
*************************************************/
#ifdef CONFIG_SMDK2412_V14
#define LCD_DEN		(1<<6)
#define LCD_DSERI	(1<<6)
#define LCD_DCLK	(1<<7)
#define LCD_RESET     	(1<<5)


#define LCD_DEN_Lo	{ gpdat = __raw_readl(S3C_GPBDAT); \
					   gpdat &= ~LCD_DEN; \
					   __raw_writel(gpdat, S3C_GPBDAT); \
					 }

#define LCD_DEN_Hi	{ gpdat = __raw_readl(S3C_GPBDAT); \
					   gpdat |= LCD_DEN; \
					   __raw_writel(gpdat, S3C_GPBDAT); \
					 }

#define LCD_DCLK_Lo	{ gpdat = __raw_readl(S3C_GPGDAT); \
					   gpdat &= ~LCD_DCLK; \
					   __raw_writel(gpdat, S3C_GPGDAT); \
					 }

#define LCD_DCLK_Hi	{ gpdat = __raw_readl(S3C_GPGDAT); \
					   gpdat |= LCD_DCLK; \
					   __raw_writel(gpdat, S3C_GPGDAT); \
					 }

#define LCD_DSERI_Lo	{ gpdat = __raw_readl(S3C_GPGDAT); \
					   gpdat &= ~LCD_DSERI; \
					   __raw_writel(gpdat, S3C_GPGDAT); \
					 }

#define LCD_DSERI_Hi	{ gpdat = __raw_readl(S3C_GPGDAT); \
					   gpdat |= LCD_DSERI; \
					   __raw_writel(gpdat, S3C_GPGDAT); \
					 }

#define LCD_RESET_Lo	{ gpdat = __raw_readl(S3C_GPGDAT); \
					   gpdat &= ~LCD_RESET; \
					   __raw_writel(gpdat, S3C_GPGDAT); \
					 }
#define LCD_RESET_Hi	{ gpdat = __raw_readl(S3C_GPGDAT); \
					   gpdat |= LCD_RESET; \
					   __raw_writel(gpdat, S3C_GPGDAT); \
					 }
#else
#define LCD_DEN		(1<<2)
#define LCD_DSERI	(1<<12)
#define LCD_DCLK	(1<<13)
#define LCD_RESET	(0)


#define LCD_DEN_Lo	{ gpdat = __raw_readl(S3C_GPGDAT); \
					   gpdat &= ~LCD_DEN; \
					   __raw_writel(gpdat, S3C_GPGDAT); \
					 }

#define LCD_DEN_Hi	{ gpdat = __raw_readl(S3C_GPGDAT); \
					   gpdat |= LCD_DEN; \
					   __raw_writel(gpdat, S3C_GPGDAT); \
					 }

#define LCD_DCLK_Lo	{ gpdat = __raw_readl(S3C_GPEDAT); \
					   gpdat &= ~LCD_DCLK; \
					   __raw_writel(gpdat, S3C_GPEDAT); \
					 }

#define LCD_DCLK_Hi	{ gpdat = __raw_readl(S3C_GPEDAT); \
					   gpdat |= LCD_DCLK; \
					   __raw_writel(gpdat, S3C_GPEDAT); \
					 }

#define LCD_DSERI_Lo  { gpdat = __raw_readl(S3C_GPEDAT); \
					   gpdat &= ~LCD_DSERI; \
					   __raw_writel(gpdat, S3C_GPEDAT); \
					 }

#define LCD_DSERI_Hi	{ gpdat = __raw_readl(S3C_GPEDAT); \
					   gpdat |= LCD_DSERI; \
  					   __raw_writel(gpdat, S3C_GPEDAT); \
					 }

#define LCD_RESET_Lo	(0)
#define LCD_RESET_Hi	(1)
#endif

void Write_LDI (u16 address, u16 data)
{
	int j;
	unsigned long DELAY = 1;
	u32 gpdat;

	LCD_DEN_Hi;		//      EN = High                                       CS high
//      LCD_DCLK_Lo;
	LCD_DCLK_Hi;		//      SCL High
	LCD_DSERI_Lo;		//      Data Low

	udelay(DELAY);

	LCD_DEN_Lo;		//      EN = Low                                CS Low
	udelay(DELAY);

	LCD_DCLK_Lo;		//      SCL Low

	udelay(DELAY);

	for (j = 7; j >= 0; j--) {
		if ((address >> j) & 0x0001) {	// DATA HIGH or LOW
			LCD_DSERI_Hi;
		}
		else {
			LCD_DSERI_Lo;
		}

		udelay(DELAY);

		LCD_DCLK_Hi;	// CLOCK = High
		udelay(DELAY);

		if (j != 0) {
			LCD_DCLK_Lo;
			udelay(DELAY);	// CLOCK = Low
		}
	}

	LCD_DSERI_Lo;		// Data Low
	udelay(DELAY);

	LCD_DEN_Hi;		//      EN = High
	udelay(DELAY);

	LCD_DEN_Lo;		//      EN = Low
	udelay(DELAY);

	LCD_DCLK_Lo;
	udelay(DELAY);		// CLOCK = Low

	for (j = 7; j >= 0; j--) {
		if ((data >> j) & 0x0001){	// DATA HIGH or LOW
			LCD_DSERI_Hi;
		}
		else {
			LCD_DSERI_Lo;
		}

		udelay(DELAY);

		LCD_DCLK_Hi;	// CLOCK = High
		udelay(DELAY);

		if (j != 0) {
			LCD_DCLK_Lo;
			udelay(DELAY);	// CLOCK = Low
		}
	}

	LCD_DEN_Hi;		// EN = High
	udelay(DELAY);
}

void SetGPIOforLDI(void)
{
	unsigned long gpdn;
	unsigned long gpdat;
	unsigned long gpcon;

#ifdef CONFIG_SMDK2412_V14
	gpdn = __raw_readl(S3C_GPBDN);
	gpdn  |= (0x1<<6);
	__raw_writel(gpdn, S3C_GPBDN);

	gpdn = __raw_readl(S3C_GPGDN);
	gpdn  |= (0x7<<5);
	__raw_writel(gpdn, S3C_GPGDN);

	gpdat = __raw_readl(S3C_GPBDAT);
	gpdat  |= (0x1<<6);
	__raw_writel(gpdat, S3C_GPBDAT);

	gpdat = __raw_readl(S3C_GPGDAT);
	gpdat  |= (0x7<<5);
	__raw_writel(gpdat, S3C_GPGDAT);

	gpcon = __raw_readl(S3C_GPBCON);
	gpcon  &= ~(0x3<<12);
	gpcon |= (0x1<<12);
	__raw_writel(gpcon, S3C_GPBCON);

	gpcon = __raw_readl(S3C_GPGCON);
	gpcon  &= ~(0x3f<<10);
	gpcon |= (0x15<<10);
	__raw_writel(gpcon, S3C_GPGCON);
#else
	gpdn = __raw_readl(S3C_GPEDN);
	gpdn  |= (0x3<<12);
	__raw_writel(gpdn, S3C_GPEDN);

	gpdn = __raw_readl(S3C_GPGDN);
	gpdn  |= (0x1<<2);
	__raw_writel(gpdn, S3C_GPGDN);

	gpdat = __raw_readl(S3C_GPEDAT);
	gpdat  |= (0x3<<12);
	__raw_writel(gpdat, S3C_GPEDAT);

	gpdat = __raw_readl(S3C_GPGDAT);
	gpdat  |= (0x1<<2);
	__raw_writel(gpdat, S3C_GPGDAT);

	gpcon = __raw_readl(S3C_GPECON);
	gpcon  &= ~(0xf<<24);
	gpcon |= (0x5<<24);
	__raw_writel(gpcon, S3C_GPECON);

	gpcon = __raw_readl(S3C_GPGCON);
	gpcon  &= ~(0x3<<4);
	gpcon |= (0x1<<4);
	__raw_writel(gpcon, S3C_GPGCON);
#endif
}

 void Init_LDI(void)
{
	u32 gpdat;

        printf("<S3C_LTS222QV> LCD will be initialized\n");

	SetGPIOforLDI();

#ifdef CONFIG_SMDK2412_V14
	LCD_RESET_Hi;
	mdelay(20);
#endif
	LCD_DEN_Hi;
	LCD_DCLK_Hi;
	LCD_DSERI_Hi;

	///////////////////////////////////////////////////////////////////
	// Power Setting Function 1
	//////////////////////////////////////////////////////////////////
	Write_LDI(0x22, 0x01);	// PARTIAL 2 DISPLAY AREA RASTER-ROW NUMBER REGISTER 1
	Write_LDI(0x03, 0x01);	// RESET REGISTER

	///////////////////////////////////////////////////////////////////
	// Initializing Function 1
	///////////////////////////////////////////////////////////////////
	Write_LDI(0x00, 0x0a);	// CONTROL REGISTER 1
	udelay(1);		// delay about 300ns
	Write_LDI(0x01, 0x10);	// CONTROL REGISTER 2
	udelay(1);		// delay about 300ns
	Write_LDI(0x02, 0x06);	// RGB INTERFACE REGISTER
	udelay(1);		// delay about 300ns
	Write_LDI(0x05, 0x00);	// DATA ACCESS CONTROL REGISTER
	udelay(1);		// delay about 300ns
	Write_LDI(0x0D, 0x00);	//

	// delay about 40ms
	mdelay(40);
	///////////////////////////////////////////////////////////////////
	// Initializing Function 2
	///////////////////////////////////////////////////////////////////
	Write_LDI(0x0E, 0x00);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x0F, 0x00);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x10, 0x00);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x11, 0x00);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x12, 0x00);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x13, 0x00);	// DISPLAY SIZE CONTROL REGISTER
	udelay(1);		// delay about 300ns
	Write_LDI(0x14, 0x00);	// PARTIAL-OFF AREA COLOR REGISTER 1
	udelay(1);		// delay about 300ns
	Write_LDI(0x15, 0x00);	// PARTIAL-OFF AREA COLOR REGISTER 2
	udelay(1);		// delay about 300ns
	Write_LDI(0x16, 0x00);	// PARTIAL 1 DISPLAY AREA STARTING REGISTER 1
	udelay(1);		// delay about 300ns
	Write_LDI(0x17, 0x00);	// PARTIAL 1 DISPLAY AREA STARTING REGISTER 2
	udelay(1);		// delay about 300ns
	Write_LDI(0x34, 0x01);	// POWER SUPPLY SYSTEM CONTROL REGISTER 14
	udelay(1);		// delay about 300ns
	Write_LDI(0x35, 0x00);	// POWER SUPPLY SYSTEM CONTROL REGISTER 7

	// delay about 30ms
	mdelay(30);

	////////////////////////////////////////////////////////////////////
	// Initializing Function 3
	////////////////////////////////////////////////////////////////////
	Write_LDI(0x8D, 0x01);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x8B, 0x28);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x4B, 0x00);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x4C, 0x00);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x4D, 0x00);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x4E, 0x00);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x4F, 0x00);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x50, 0x00);	//  ID CODE REGISTER 2
	// delay about 50 us
	udelay(50);

	Write_LDI(0x86, 0x00);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x87, 0x26);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x88, 0x02);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x89, 0x05);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x33, 0x01);	//  POWER SUPPLY SYSTEM CONTROL REGISTER 13
	udelay(1);		// delay about 300ns
	Write_LDI(0x37, 0x06);	//  POWER SUPPLY SYSTEM CONTROL REGISTER 12

	// delay about 50 us
	udelay(50);

	Write_LDI(0x76, 0x00);	//  SCROLL AREA START REGISTER 2

	// delay about 30ms
	mdelay(30);
	/////////////////////////////////////////////////////////////////////
	// Initializing Function 4
	/////////////////////////////////////////////////////////////////////
	Write_LDI(0x42, 0x00);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x43, 0x00);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x44, 0x00);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x45, 0x00);	//  CALIBRATION REGISTER
	udelay(1);		// delay about 300ns
	Write_LDI(0x46, 0xef);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x47, 0x00);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x48, 0x00);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x49, 0x01);	//  ID CODE REGISTER 1               check it out

	// delay about 50 us
	udelay(50);

	Write_LDI(0x4A, 0x3f);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x3C, 0x00);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x3D, 0x00);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x3E, 0x01);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x3F, 0x3f);	//
	udelay(1);		// delay about 300ns
//      Write_LDI(0x40,0x03);  //       horizontal back porch
	Write_LDI(0x40, 0x01);	//       horizontal back porch    //050105 Boaz.Kim
	udelay(1);		// delay about 300ns
//      Write_LDI(0x41,0x04);  //       vertical back porch
	Write_LDI(0x41, 0x0a);	//       horizontal back porch   //050105 Boaz.Kim
	udelay(1);		// delay about 300ns
	Write_LDI(0x8F, 0x05);	//

	// delay about 30ms
	mdelay(30);

	/////////////////////////////////////////////////////////////////////
	// Initializing Function 5
	/////////////////////////////////////////////////////////////////////
	Write_LDI(0x90, 0x05);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x91, 0x44);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x92, 0x44);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x93, 0x44);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x94, 0x33);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x95, 0x05);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x96, 0x05);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x97, 0x44);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x98, 0x44);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x99, 0x44);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x9A, 0x33);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x9B, 0x33);	//
	udelay(1);		// delay about 300ns
	Write_LDI(0x9C, 0x33);	//
	udelay(1);		// delay about 300ns

#if 1
	Write_LDI(0x9D, 0x80);	//       16 or 18bit RGB
#else
	Write_LDI(0x9D, 0x81);	//       6bit RGB
#endif

	// delay about 30ms
	mdelay(30);
	/////////////////////////////////////////////////////////////////////
	// Power Setting 2
	/////////////////////////////////////////////////////////////////////
	Write_LDI(0x1D, 0x08);	//

	// delay about 30ms
	mdelay(30);

	Write_LDI(0x23, 0x00);	//  PARTIAL 2 DISPLAY AREA RASTER-ROW NUMBER REGISTER 2
	// delay about 50 us
	udelay(50);
	Write_LDI(0x24, 0x94);	//  POWER SUPPLY SYSTEM CONTROL REGISTER 1
	// delay about 50 us
	udelay(50);
	Write_LDI(0x25, 0x6f);	//  POWER SUPPLY SYSTEM CONTROL REGISTER 2

	// delay about 40ms
	mdelay(40);


	/////////////////////////////////////////////////////////////////////
	// Power Setting 3
	/////////////////////////////////////////////////////////////////////
	Write_LDI(0x28, 0x1e);	//
	Write_LDI(0x1A, 0x00);	//
	Write_LDI(0x21, 0x10);	//  PARTIAL 1 DISPLAY AREA RASTER-ROW NUMBER REGISTER 2
	Write_LDI(0x18, 0x25);	//  PARTIAL 2 DISPLAY AREA STARTING REGISTER 1

	// delay about 40ms
	mdelay(40);

	Write_LDI(0x19, 0x48);	//  PARTIAL 2 DISPLAY AREA STARTING REGISTER 2
	Write_LDI(0x18, 0xe5);	//  PARTIAL 2 DISPLAY AREA STARTING REGISTER 1

	// delay about 10ms
	mdelay(10);

	Write_LDI(0x18, 0xF7);	//  PARTIAL 2 DISPLAY AREA STARTING REGISTER 1

	// delay about 40ms
	mdelay(40);

	Write_LDI(0x1B, 0x07);	// org
//      Write_LDI(0x1B,0x01);  // 90 rotate
//      Write_LDI(0x1B,0x02);  // 90 rotate
//      Write_LDI(0x1B,0x03);  // 90 rotate


	// delay about 80ms
	mdelay(80);

	Write_LDI(0x1F, 0x6b);	// org
//      Write_LDI(0x1F,0x5E);  // 90 rotate

	Write_LDI(0x20, 0x51);	//  org, PARTIAL 1 DISPLAY AREA RASTER-ROW NUMBER REGISTER 1
//      Write_LDI(0x20,0x5F);  //  90 rotate, PARTIAL 1 DISPLAY AREA RASTER-ROW NUMBER REGISTER 1

	Write_LDI(0x1E, 0xc1);	//

	// delay about 10ms
	mdelay(10);

	Write_LDI(0x21, 0x00);	//  PARTIAL 1 DISPLAY AREA RASTER-ROW NUMBER REGISTER 2
	Write_LDI(0x3B, 0x01);	//

	// delay about 20ms
	mdelay(20);

	Write_LDI(0x00, 0x20);	//  CONTROL REGISTER 1
	Write_LDI(0x02, 0x01);	//  RGB INTERFACE REGISTER

	// delay about 10ms
	mdelay(10);
//      Reg16_OPCLK_DIV = 0x0201;                               // 6.4

}
#endif

#ifdef BOARD_LATE_INIT
int board_late_init (void)
{
	uint *magic = (uint*)(PHYS_SDRAM_1);
	char boot_cmd[100];

	if ((0x24564236 == magic[0]) && (0x20764316 == magic[1])) {
		sprintf(boot_cmd, "nand erase 0 40000;nand write %08x 0 40000", PHYS_SDRAM_1+0x8000);
		magic[0] = 0;
		magic[1] = 0;
		printf("\n\nready for self-burning U-Boot image\n\n");
		setenv("bootdelay", "0");
		setenv("bootcmd", boot_cmd);
	}

#ifdef CONFIG_LCD_2412
	Init_LDI();
#endif

	return 0;

}
#endif

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	vu_long *mem_reg = (vu_long*) 0x48000000;

	printf("Board: SMDK2412  ");
	switch ((*mem_reg>>2) & 0x3) {
        case 0:
                puts("SDRAM ");
                break;

        case 1:
                puts("Mobile SDRAM ");
                break;

        case 2:
        case 3:
                puts("Mobile DDR ");
                break;

        default:
                puts("unknown Memory Type \n");
        }

	switch ((*mem_reg) & 0x1) {
	case 1:
		puts("16bits \n");
		break;
	case 0:
	default:
		puts("32bits \n");

	}
	return (0);
}
#endif

#ifdef CONFIG_ENABLE_MMU
ulong virt_to_phy_smdk2412(ulong addr)
{
	if ((0xc0000000 <= addr) && (addr < 0xc4000000))
		return (addr - 0xc0000000 + 0x30000000);
	else
		printf("do not support this address : %08lx\n", addr);

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
