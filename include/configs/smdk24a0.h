/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Gary Jennejohn <gj@denx.de>
 * David Mueller <d.mueller@elsoft.ch>
 *
 * Configuation settings for the SAMSUNG SMDK2410 board.
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

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define	CONFIG_S3C24A0		1		/* in a SAMSUNG S3C24A0 SoC     */
#define	CONFIG_S3C24XX		1		/* in a SAMSUNG S3C24XX Family  */
#define CONFIG_SMDK24A0		1		/* on a SAMSUNG SMDK24A0 Board  */

/* input clock of PLL */
#define CONFIG_SYS_CLK_FREQ	12000000	/* the SMDK24A0 has 12MHz input clock */

#undef CONFIG_USE_IRQ				/* we don't need IRQ/FIQ stuff */

/*
 * Architecture magic and machine type
 */
#define MACH_TYPE		495
#define UBOOT_MAGIC		(0x43090000 | MACH_TYPE)

/* Power Management is enabled */
#define CONFIG_PM

#define CONFIG_SHOW_CPUINFO

#undef	CONFIG_USE_NOR_BOOT

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */
#define CONFIG_DRIVER_CS8900	1	/* we have a CS8900 on-board */
#define CS8900_BASE		(0x04000300 | (1<<24))
#define CS8900_BUS16		1 /* the Linux driver does accesses as shorts */

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1          1	/* we use SERIAL 1 on SMDK2410 */


#undef CONFIG_S3C24XX_I2C		/* this board has H/W I2C */
#ifdef CONFIG_S3C24XX_I2C
#define CONFIG_HARD_I2C		1
#define	CFG_I2C_SPEED	50000
#define	CFG_I2C_SLAVE	0xFE
#endif

#define CONFIG_DOS_PARTITION
#define CONFIG_SUPPORT_VFAT

#define CONFIG_USB_OHCI
#define CONFIG_USB_STORAGE
#define CONFIG_S3C24XX_USBD

#define USBD_DOWN_ADDR		PHYS_SDRAM_1
#define USBD_DETECT_IRQ()	(SRCPND_REG & (BIT_USBD))
#define USBD_CLEAR_IRQ()	do { SRCPND_REG = (BIT_USBD); \
					INTPND_REG = (BIT_USBD); \
				} while(0)


/************************************************************
 * RTC
 ************************************************************/
#define	CONFIG_RTC_S3C24XX	1

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		115200

/***********************************************************
 * Command definition
 ***********************************************************/
#define CONFIG_COMMANDS \
			(CONFIG_CMD_DFL	| \
			CFG_CMD_CACHE	| \
			CFG_CMD_NAND	| \
			/*CFG_CMD_EEPROM |*/ \
			/*CFG_CMD_I2C	 |*/ \
			CFG_CMD_USB	| \
			CFG_CMD_REGINFO	| \
			CFG_CMD_DATE	| \
			CFG_CMD_PING	| \
			CFG_CMD_ELF)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CONFIG_BOOTDELAY	3
/*#define CONFIG_BOOTARGS    	"root=ramfs devfs=mount console=ttySA0,9600" */
#define CONFIG_ETHADDR		00:40:5c:26:0a:5b
#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_IPADDR		192.168.10.1
#define CONFIG_SERVERIP		192.168.10.147

#define CONFIG_ZERO_BOOTDELAY_CHECK
#define CONFIG_BOOTCOMMAND	"md 10000000"

/*#define CONFIG_NET_MULTI	1 */

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200		/* speed to run kgdb serial port */
/* what's this ? it's not used anywhere */
#define CONFIG_KGDB_SER_INDEX	1		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory		*/
#define	CFG_PROMPT		"SMDK24A0 # "	/* Monitor Command Prompt	*/
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x10000000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x13e00000	/* 63 MB in DRAM	*/

#define	CFG_LOAD_ADDR		0x10000000	/* default load address	*/

/* the PWM TImer 4 uses a counter of 15625 for 10 ms, so we need */
/* it to wrap 100 times (total 1562500) to get 1 sec. */
#define	CFG_HZ			1562500

/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x10000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x04000000 /* 64 MB */

#define CFG_FLASH_BASE		0x00000000

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	1024
#define CONFIG_AMD_LV800
#define PHYS_FLASH_SIZE		0x100000
#define CFG_ENV_ADDR		0

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(5*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(5*CFG_HZ) /* Timeout for Flash Write */

#define	CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector */

/*
 * SMDK24A0 board specific data
 */

#define CONFIG_IDENT_STRING	" for SMDK24A0"

/* total memory required by uboot */
#define UBOOT_SIZE		(2*1024*1024)
/* size of vivi binary */
#define MAX_UBOOT_BIN_SIZE	(256*1024)
#define MMU_TLB_SIZE		(16*1024)
#define HEAP_SIZE		(1*1024*1024)
#define STACK_SIZE		(512*1024)

/* base address for uboot */
#define UBOOT_BASE		(DRAM_BASE + DRAM_SIZE - UBOOT_SIZE)
#define MMU_TLB_BASE		(UBOOT_BASE + MAX_UBOOT_BIN_SIZE)
#define HEAP_BASE		(UBOOT_BASE + (512*1024))
#define STACK_BASE		(UBOOT_BASE + UBOOT_SIZE - STACK_SIZE)

#define RAM_SIZE		(UBOOT_BASE - DRAM_BASE)
#define RAM_BASE		DRAM_BASE

#define DRAM_BASE		DRAM_BASE0
#define DRAM_SIZE		SZ_64M

/*
 * NAND Conf for Our Command
 */
#define NF_nFCE_L()		do { NFCONT_REG&=~(1<<7); } while(0)
#define NF_nFCE_H()		do { NFCONT_REG|=(1<<7); } while(0)
#define NF_RSTECC()		do { NFCONT_REG|=(1<<8); } while(0)
#define NF_TRANSRnB()		do { while(!(NFSTAT_REG&(1<<10))); } while(0)
#define NF_CLRRnB()		do { (NFSTAT_REG|=(1<<13)); } while(0)
#define NF_MECC_Lock()		do {  NFCONT_REG|=(1<<9); } while(0)
#define NF_MECC_UnLock()	do {  NFCONT_REG&=~(1<<9); } while(0)

/* NAND configuration */
#define CFG_MAX_NAND_DEVICE	1

#define CFG_NAND_BASE		0x40c00000
#define NAND_MAX_FLOORS		1
#define NAND_MAX_CHIPS		1
#define NAND_ChipID_UNKNOWN	0x00
#define SECTORSIZE		512

#define ADDR_COLUMN		1
#define ADDR_PAGE		2
#define ADDR_COLUMN_PAGE	3

#define CFG_NAND_CE			;
#define CFG_NAND_CLE			;
#define CFG_NAND_ALE			;
#define CFG_NAND_RDY			;
#define NAND_DISABLE_CE(nand)		(NFCONT_REG |= (1 << 7))
#define NAND_ENABLE_CE(nand)		(NFCONT_REG &= ~(1 << 7))
#define NAND_CTL_CLRALE(nandptr)	;
#define NAND_CTL_SETALE(nandptr)	;
#define NAND_CTL_CLRCLE(nandptr)	;
#define NAND_CTL_SETCLE(nandptr)	;
#define NAND_WAIT_READY(nand)		do { while(!(NFSTAT_REG & (1 << 10)));} while(0)
#define WRITE_NAND_COMMAND(d, adr)	do {*(volatile u8 *)((ulong)adr+NFCMD_OFFSET) = d;} while(0)
#define WRITE_NAND_ADDRESS(d, adr)	do {*(volatile u8 *)((ulong)adr+NFADDR_OFFSET) = d;} while(0)
#define WRITE_NAND(d, adr)		do {*(volatile u8 *)((ulong)adr+NFDATA_OFFSET) = d;} while(0)
#define READ_NAND(adr)			(*(volatile u8 *)((ulong)adr+NFDATA_OFFSET))

#define CFG_NAND_SKIP_BAD_DOT_I		1  /* ".i" read skips bad blocks   */

#define	CFG_NAND_WP			1
#define	NAND_WP_OFF()			do { \
						GPCON_L_REG =  (GPCON_L_REG & ~(0x3<<2))|(0x1<<2); \
						GPDAT_REG |=(1<<1); \
						NFCONT_REG|=(1<<8); \
						NFCONT_REG&=~(1<<9); \
						NFCONT_REG&=~(1<<2); \
					} while(0)
#define	NAND_WP_ON()			do { \
						NFCONT_REG|=(1<<9); \
						GPCON_L_REG =  (GPCON_L_REG & ~(0x3<<2))|(0x3<<2); \
					} while(0)

#define CFG_ENV_NAND_BLOCK	8

#define NAND_OOB_SIZE           (16)
#define NAND_PAGES_IN_BLOCK     (32)
#define NAND_PAGE_SIZE          (512)
#define NAND_BLOCK_SIZE         (NAND_PAGE_SIZE*NAND_PAGES_IN_BLOCK)
#define NAND_BLOCK_CNT          (4096)
#define NAND_BLOCK_MASK         (NAND_BLOCK_SIZE - 1)
#define NAND_PAGE_MASK          (NAND_PAGE_SIZE - 1)

/* mark bad block */
#define NF_BB_ON   1
#define NF_BB_OFF  0

/* NAND R/W status */
#define NF_RW_NORMAL   1   /* default */
#define NF_RW_YAFFS    2   /* yaffs image r/w */

/* NAND R/W ECC status */
#define NF_USE_ECC     1
#define NF_USE_MTD_ECC 2


#define CFG_ENV_IS_IN_NAND
#define CFG_ENV_OFFSET 0x0003C000

#endif	/* __CONFIG_H */

