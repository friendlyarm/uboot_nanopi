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
#if 1
	SMBIDCYR1_REG = 0;			//Bank1 Idle cycle ctrl.
	SMBWSTWRR1_REG = 14;			//Bank1 Write Wait State ctrl.
	SMBWSTOENR1_REG = 2;			//Bank1 Output Enable Assertion Delay ctrl.     Tcho?
	SMBWSTWENR1_REG = 2;			//Bank1 Write Enable Assertion Delay ctrl.
	SMBWSTRDR1_REG = 14;			//Bank1 Read Wait State cont. = 14 clk	    Tacc?
#endif
	SMBCR1_REG |=  ((1<<15)|(1<<7));		//dsf
	SMBCR1_REG |=  ((1<<2)|(1<<0));		//SMWAIT active High, Read Byte Lane Enabl	    WS1?
	SMBCR1_REG &= ~((3<<20)|(3<<12));	//SMADDRVALID = always High when Read/Write
	SMBCR1_REG &= ~(3<<4);			//Clear Memory Width
	SMBCR1_REG |=  (1<<4);			//Memory Width = 16bit
}

static void usb_pre_init (void)
{
	CLKDIV1CON_REG |= 1<<4;

	USB_RSTCON_REG = 0x1;
	delay(500);
	USB_RSTCON_REG = 0x2;
	delay(500);
	USB_RSTCON_REG = 0x0;
	delay(500);

//	USB_PHYCTRL_REG &= ~0x2;
	USB_CLKCON_REG |= 0x2;
}

#if 0
#ifdef CONFIG_ONENAND
static void onenand_pre_init(void)
{
	u16 sysconfig1;
	u16* addr = (u16*)(CFG_ONENAND_BASE+ONENAND_REG_SYS_CFG1);

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
#endif

#if 0
/*
 * When NAND is not used as Boot Device
 */
static void nand_pre_init(void)
{
	SMBIDCYR1_REG = 0xf;
	SMBWSTRDR1_REG = 0x1f;
	SMBWSTWRR1_REG = 0x1f;
	SMBWSTOENR1_REG = 0;
	SMBWSTWENR1_REG = 1;
	SMBCR1_REG = 0x00303000;
	SMBSR1_REG = 0;
	SMBWSTBDR1_REG = 0x1f;
}
#endif

int board_init(void)
{
//	nand_pre_init();
	cs8900_pre_init();
	usb_pre_init();
#ifdef CONFIG_ONENAND
//	onenand_pre_init();
#endif

//	SMBCR4_REG = 0x00000015;
//	SMBCR1_REG = 0x00000015;
	gd->bd->bi_arch_number = MACH_TYPE;
	gd->bd->bi_boot_params = (PHYS_SDRAM_1+0x100);

#if 0
	icache_enable();
	dcache_enable();
#endif

	return 0;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

#ifdef CONFIG_LCD
static void SetLcdPort(void)
{
//	ulong gpdn;
	ulong gpdat;
	ulong gpcon;
	ulong hclkcon; //, sclkcon;

	/* Enable clock to LCD */
	hclkcon = readl(HCLKCON);
	hclkcon |= (1<<10);
	writel(hclkcon, HCLKCON);

	// To select TFT LCD type
	gpdat = readl(MISCCR);
	gpdat  |= (1<<28);
	writel(gpdat, MISCCR);

	writel(0xaaaaaaaa, GPCCON);  // CTRL,  VD[7:0]
	writel(0xaaaaaaaa, GPDCON);  // VD[23:8]

	gpcon = readl(GPBCON);
	gpcon  = (gpcon & ~(3<<8)) | (1<<8); // Backlight Pwm control
	writel(gpcon, GPBCON);

	gpdat = readl(GPBDAT);
	gpdat  |= (1<<8);
	writel(gpdat, GPBDAT);

	// LCD _nRESET control
	gpcon = readl(GPBCON);
	gpcon  = (gpcon & ~(3<<2)) |(1<<2);
	writel(gpcon, GPBCON);

	gpdat = readl(GPBDAT);
	gpdat  |= (1<<1);
	writel(gpdat, GPBDAT);
}

#define LCD_DEN		(1<<14)
#define LCD_DSERI	(1<<11)
#define LCD_DCLK	(1<<10)

#define LCD_DEN_BIT	14
#define LCD_DSERI_BIT	11
#define LCD_DCLK_BIT	10

#define LCD_nRESET	1

#define LCD_RESET	(0)
#define LCD_RESET_Lo	(0)
#define LCD_RESET_Hi	(1)

#define LCD_DEN_Lo	{ \
			  gpdat = readl(GPLDAT); \
			  gpdat &= ~LCD_DEN; \
			  writel(gpdat, GPLDAT); \
			}

#define LCD_DEN_Hi	{ \
			  gpdat = readl(GPLDAT); \
			  gpdat |= LCD_DEN; \
			  writel(gpdat, GPLDAT); \
			}

#define LCD_DCLK_Lo	{ \
			  gpdat = readl(GPLDAT); \
			  gpdat &= ~LCD_DCLK; \
			  writel(gpdat, GPLDAT); \
			}

#define LCD_DCLK_Hi	{ \
			  gpdat = readl(GPLDAT); \
			  gpdat |= LCD_DCLK; \
			  writel(gpdat, GPLDAT); \
			}

#define LCD_DSERI_Lo	{ \
			  gpdat = readl(GPLDAT); \
			  gpdat &= ~LCD_DSERI; \
			  writel(gpdat, GPLDAT); \
			}

#define LCD_DSERI_Hi	{ \
			  gpdat = readl(GPLDAT); \
			  gpdat |= LCD_DSERI; \
			  writel(gpdat, GPLDAT); \
			}


#define SET_LCD_DATA	{ \
			  gpdat= readl(GPLCON); \
			  gpdat &= ~(((3<<(LCD_DEN_BIT*2))) | ((3<<(LCD_DCLK_BIT*2))) | ((3<<(LCD_DSERI_BIT*2)))); \
			  gpdat |= (((1<<(LCD_DEN_BIT*2))) | ((1<<(LCD_DCLK_BIT*2))) | ((1<<(LCD_DSERI_BIT*2)))); \
			  writel(gpdat, GPLCON); \
			  gpfpu = readl(GPLPU); \
			  gpfpu |= ~(((3<<(LCD_DEN_BIT*2))) | ((3<<(LCD_DCLK_BIT*2))) | ((3<<(LCD_DSERI_BIT*2)))); \
			  gpfpu |= (((2<<(LCD_DEN_BIT*2))) | ((2<<(LCD_DCLK_BIT*2))) | ((2<<(LCD_DSERI_BIT*2)))); \
			  writel(gpfpu, GPLPU); \
			}



static void delayLoop(int count)
{
    int j;
    for(j = 0; j < count; j++)  ;
}

static void Write_LDI_LTV350(int address, int data)
{
 	unchar	dev_id_code=0x1D;
	int     j;
	unchar DELAY=50;
	ulong gpdat;

	LCD_DEN_Hi; 		// EN = High CS high
	LCD_DCLK_Hi;		// SCL High
	LCD_DSERI_Hi;		// Data Low
	delayLoop(DELAY);

	LCD_DEN_Lo; 		// EN = Low CS Low
	delayLoop(DELAY);

	for (j = 5; j >= 0; j--) {
		LCD_DCLK_Lo;	// SCL Low

		if ((dev_id_code >> j) & 0x0001) {	// DATA HIGH or LOW
			LCD_DSERI_Hi;
		}
		else {
			LCD_DSERI_Lo;
		}
		delayLoop(DELAY);

		LCD_DCLK_Hi;			// CLOCK = High
		delayLoop(DELAY);
	}

	// RS = "0" : index data
	LCD_DCLK_Lo;			// CLOCK = Low
	LCD_DSERI_Lo;
	delayLoop(DELAY);
	LCD_DCLK_Hi;			// CLOCK = High
	delayLoop(DELAY);

	// Write
	LCD_DCLK_Lo;			// CLOCK = Low
	LCD_DSERI_Lo;
	delayLoop(DELAY);
	LCD_DCLK_Hi;			// CLOCK = High
	delayLoop(DELAY);

	for (j = 15; j >= 0; j--) {
		LCD_DCLK_Lo;	//	SCL Low

		if ((address >> j) & 0x0001) {	// DATA HIGH or LOW
			LCD_DSERI_Hi;
		}
		else {
			LCD_DSERI_Lo;
		}

		delayLoop(DELAY);

		LCD_DCLK_Hi;	// CLOCK = High
		delayLoop(DELAY);

	}
	LCD_DSERI_Hi;
	delayLoop(DELAY);

	LCD_DEN_Hi; 		// EN = High
	delayLoop(DELAY*10);

	LCD_DEN_Lo; 		// EN = Low CS Low
	delayLoop(DELAY);

	for (j = 5; j >= 0; j--) {
		LCD_DCLK_Lo;	// SCL Low

		if ((dev_id_code >> j) & 0x0001) {	// DATA HIGH or LOW
			LCD_DSERI_Hi;
		}
		else {
			LCD_DSERI_Lo;
		}
		delayLoop(DELAY);

		LCD_DCLK_Hi;			// CLOCK = High
		delayLoop(DELAY);

	}

	// RS = "1" instruction data
	LCD_DCLK_Lo;			// CLOCK = Low
	LCD_DSERI_Hi;
	delayLoop(DELAY);
	LCD_DCLK_Hi;			// CLOCK = High
	delayLoop(DELAY);

	// Write
	LCD_DCLK_Lo;			// CLOCK = Low
	LCD_DSERI_Lo;
	delayLoop(DELAY);
	LCD_DCLK_Hi;			// CLOCK = High
	delayLoop(DELAY);

	for (j = 15; j >= 0; j--) {
		LCD_DCLK_Lo;		//	SCL Low

		if ((data >> j) & 0x0001) {	// DATA HIGH or LOW
			LCD_DSERI_Hi;
		}
		else {
			LCD_DSERI_Lo;
		}
		delayLoop(DELAY);

		LCD_DCLK_Hi;			// CLOCK = High
		delayLoop(DELAY);
	}

	LCD_DEN_Hi; 				// EN = High
	delayLoop(DELAY);
}


static void Init_LDI(void)
{
	unsigned long gpdat, gpfpu; // , lcdcon1

	printf("LCD TYPE :: S3C_LTV350QV LCD will be initialized\n");
	SetLcdPort();

	// LCD module reset
	gpdat = readl(GPBDAT);
	gpdat  |= (1<<(LCD_nRESET*2));
	writel(gpdat, GPBDAT);

	gpdat = readl(GPBDAT);
	gpdat  &= ~(1<<(LCD_nRESET*2));   // goes to LOW
	writel(gpdat, GPBDAT);
	udelay(10000);

	gpdat = readl(GPBDAT);
	gpdat  |= (1<<(LCD_nRESET*2));   // goes to HIGH
	writel(gpdat, GPBDAT);

	SET_LCD_DATA;

	udelay(10000);
	LCD_DEN_Hi;
	LCD_DCLK_Hi;
	LCD_DSERI_Hi;

	Write_LDI_LTV350(0x01,0x001d);
	Write_LDI_LTV350(0x02,0x0000);
    	Write_LDI_LTV350(0x03,0x0000);
    	Write_LDI_LTV350(0x04,0x0000);
    	Write_LDI_LTV350(0x05,0x50a3);
    	Write_LDI_LTV350(0x06,0x0000);
    	Write_LDI_LTV350(0x07,0x0000);
    	Write_LDI_LTV350(0x08,0x0000);
   	Write_LDI_LTV350(0x09,0x0000);
   	Write_LDI_LTV350(0x0a,0x0000);
   	Write_LDI_LTV350(0x10,0x0000);
   	Write_LDI_LTV350(0x11,0x0000);
   	Write_LDI_LTV350(0x12,0x0000);
   	Write_LDI_LTV350(0x13,0x0000);
   	Write_LDI_LTV350(0x14,0x0000);
   	Write_LDI_LTV350(0x15,0x0000);
   	Write_LDI_LTV350(0x16,0x0000);
   	Write_LDI_LTV350(0x17,0x0000);
   	Write_LDI_LTV350(0x18,0x0000);
   	Write_LDI_LTV350(0x19,0x0000);

	udelay(10000);

	Write_LDI_LTV350(0x09,0x4055);
	Write_LDI_LTV350(0x0a,0x0000);

	udelay(10000);

	Write_LDI_LTV350(0x0a,0x2000);

	udelay(50000);

	Write_LDI_LTV350(0x01,0x409d);
	Write_LDI_LTV350(0x02,0x0204);
	Write_LDI_LTV350(0x03,0x2100);
	Write_LDI_LTV350(0x04,0x1000);
	Write_LDI_LTV350(0x05,0x5003);
	Write_LDI_LTV350(0x06,0x0009);	//vbp
	Write_LDI_LTV350(0x07,0x000f);	//hbp
	Write_LDI_LTV350(0x08,0x0800);
	Write_LDI_LTV350(0x10,0x0000);
	Write_LDI_LTV350(0x11,0x0000);
	Write_LDI_LTV350(0x12,0x000f);
	Write_LDI_LTV350(0x13,0x1f00);
	Write_LDI_LTV350(0x14,0x0000);
	Write_LDI_LTV350(0x15,0x0000);
	Write_LDI_LTV350(0x16,0x0000);
	Write_LDI_LTV350(0x17,0x0000);
	Write_LDI_LTV350(0x18,0x0000);
	Write_LDI_LTV350(0x19,0x0000);

	udelay(50000);

	Write_LDI_LTV350(0x09,0x4a55);
	Write_LDI_LTV350(0x0a,0x2000);
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

//	Init_LDI();

	return 0;
}
#endif

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	vu_long *mem_reg = (vu_long*) 0x48000000;

	printf("Board: SMDK2443 ");
	switch ((*mem_reg>>2) & 0x3) {
	case 0:
		puts("SDRAM\n");
		break;

	case 1:
		puts("Mobile SDRAM\n");
		break;

	case 2:
	case 3:
		puts("Mobile DDR\n");
		break;

	default:
		puts("unknown Memory Type\n");
	}
	return (0);
}
#endif

#ifdef CONFIG_ENABLE_MMU
ulong virt_to_phy_smdk2443(ulong addr)
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
