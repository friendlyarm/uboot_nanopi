/*
 * (C) Copyright 2005-2006
 * Seung-Chull, Suh <sc.suh@samsung.com>
 *
 * Configuation settings for the SAMSUNG SMDK2412/3 board.
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
#define	CONFIG_S3C2412		1		/* in a SAMSUNG S3C2412/3 SoC     */
#define	CONFIG_S3C24XX		1		/* in a SAMSUNG S3C24XX Family  */
#define CONFIG_SMDK2412		1		/* on a SAMSUNG SMDK2412 Board  */
//#define CONFIG_SMDK2412_V14	1		/* SMDK2412 Board version above 1.4 */

/* input clock of PLL */
#define CONFIG_SYS_CLK_FREQ	12000000	/* the SMDK2412 has 12MHz input clock */

/* VDD ARM voltage of S3C2412 */
#define VDD_ARM			140		/* VDDarm = 1.4V(default 266Mhz) on SMDK2412 */
//#define VDD_ARM		130		/* VDDarm = 1.3V(default 200Mhz) on SMDK2412 */

#define CONFIG_ENABLE_MMU
#ifdef CONFIG_ENABLE_MMU
#define virt_to_phys(x)	virt_to_phy_smdk2412(x)
#else
#define virt_to_phys(x)	(x)
#endif

#define CONFIG_MEMORY_UPPER_CODE

#undef CONFIG_USE_IRQ				/* we don't need IRQ/FIQ stuff */

#define CONFIG_INCLUDE_TEST

#define CONFIG_ZIMAGE_BOOT
#define CONFIG_IMAGE_BOOT

#define BOARD_LATE_INIT

#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG

/*
 * Architecture magic and machine type
 */
#define MACH_TYPE		1009
#define UBOOT_MAGIC		(0x43090000 | MACH_TYPE)

/* Power Management is enabled */
#define CONFIG_PM

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#undef	CONFIG_SKIP_RELOCATE_UBOOT
#undef	CONFIG_USE_NOR_BOOT
/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 1024*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

#define CFG_STACK_SIZE		512*1024
/*
 * Hardware drivers
 */
#define CONFIG_DRIVER_CS8900	1	/* we have a CS8900 on-board */
#ifdef CONFIG_SMDK2412_V14
#define CS8900_BASE		(0x29000300)
#define CS8900_BUS16		1 /* the Linux driver does accesses as shorts */
#else
#define CS8900_BASE		(0x2a000600)
#define CS8900_BUS16		1 /* the Linux driver does accesses as shorts */
#endif	/* CONFIG_SMDK2412_V14 */

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1          1	/* we use SERIAL 1 on SMDK2412 */

#define	CFG_HUSH_PARSER			/* use "hush" command parser	*/
#ifdef	CFG_HUSH_PARSER
#define	CFG_PROMPT_HUSH_PS2	"> "
#endif

#define CONFIG_CMDLINE_EDITING

#undef CONFIG_S3C24XX_I2C		/* this board has H/W I2C */
#ifdef CONFIG_S3C24XX_I2C
#define CONFIG_HARD_I2C		1
#define	CFG_I2C_SPEED		50000
#define	CFG_I2C_SLAVE		0xFE
#endif

#define CONFIG_DOS_PARTITION
#define CONFIG_SUPPORT_VFAT

#define CONFIG_USB_OHCI
#define CONFIG_USB_STORAGE
#define CONFIG_S3C_USBD

#define USBD_DOWN_ADDR		0xc0000000

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
			CFG_CMD_MMC	| \
			CFG_CMD_USB	| \
			CFG_CMD_REGINFO	| \
			CFG_CMD_LOADS	| \
			CFG_CMD_LOADB	| \
			CFG_CMD_DATE	| \
			CFG_CMD_JFFS2	| \
			CFG_CMD_FAT	| \
			CFG_CMD_PING	| \
			CFG_CMD_ELF)	\
			& ~(CFG_CMD_AUTOSCRIPT	| \
				CFG_CMD_BOOTD	| \
				CFG_CMD_IMI	| \
				CFG_CMD_RUN	| \
				CFG_CMD_IMLS	| \
				CFG_CMD_CONSOLE	| \
				0)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CONFIG_BOOTDELAY	3
/*#define CONFIG_BOOTARGS    	"root=ramfs devfs=mount console=ttySA0,9600" */
#define CONFIG_ETHADDR		00:40:5c:26:0a:5b
#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_IPADDR		192.168.0.20
#define CONFIG_SERVERIP		192.168.0.10

#define CONFIG_ZERO_BOOTDELAY_CHECK
#define CONFIG_BOOTCOMMAND	"nand read c0008000 40000 1c0000;bootm c0008000"

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
#ifdef CONFIG_SMDK2412_V14
#define	CFG_PROMPT		"SMDK2412(1.4) # "	/* Monitor Command Prompt	*/
#else
#define	CFG_PROMPT		"SMDK2412 # "	/* Monitor Command Prompt	*/
#endif
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CFG_PBSIZE		384		/* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x30000000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x33e00000	/* 63 MB in DRAM	*/

#undef  CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define	CFG_LOAD_ADDR		0x30000000	/* default load address	*/

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
#define CONFIG_STACKSIZE	0x40000		/* regular stack 256KB */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_SDRAM_AUTO_DETECT
#define CONFIG_CLK_266_133_66
//#define CONFIG_SDRAM_16BITS

#ifdef CONFIG_SDRAM_AUTO_DETECT
 #define	CFG_BANK_CFG_VAL	0x0004925D
 #ifndef CONFIG_CLK_266_133_66
   #define	CFG_BANK_CON2_VAL	0x0046003A
   #define	CFG_BANK_CON2_VAL_ALT	0x0057003A
   #define	CFG_BANK_REFRESH_VAL	0x00000313
 #else
   #define	CFG_BANK_CON2_VAL	0x0058003A
   #define	CFG_BANK_CON2_VAL_ALT	0x0069003A
   #define	CFG_BANK_REFRESH_VAL	0x0000040D
 #endif /* CONFIG_CLK_266_133_66 */

 #ifndef CONFIG_SDRAM_16BITS
   #define	CFG_BANK_CFG_VAL_ALT	0x00048954
 #else
   #define	CFG_BANK_CFG_VAL_ALT	0x00048955
 #endif
#else
#endif /*CONFIG_SDRAM_AUTO_DETECT */

#define	CFG_BANK_CON1_VAL	0x040000D0
#define	CFG_BANK_CON3_VAL	0x80000030

#define CONFIG_NR_DRAM_BANKS	1	   /* we have 2 bank of DRAM */
#define PHYS_SDRAM_1		0x30000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x04000000 /* 64 MB */

#define CFG_FLASH_BASE		0x00000000

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	1024
#define CONFIG_AMD_LV800
#define PHYS_FLASH_SIZE		0x100000

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(5*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(5*CFG_HZ) /* Timeout for Flash Write */

#define CFG_ENV_ADDR		0
#define CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector */

/*
 * SMDK2412 board specific data
 */

#define CONFIG_IDENT_STRING	" for SMDK2412"

/* total memory required by uboot */
#define CFG_UBOOT_SIZE		(2*1024*1024)

/* base address for u-boot */
#ifdef CONFIG_ENABLE_MMU
#define CFG_UBOOT_BASE		0xc3e00000
#define CFG_PHY_UBOOT_BASE	0x33e00000
#else
#define CFG_UBOOT_BASE		0x33e00000
#define CFG_PHY_UBOOT_BASE	0x33e00000
#endif

/* NAND configuration */
#define CFG_MAX_NAND_DEVICE	1

#define CFG_NAND_BASE		(0x4e000010)
#define NAND_MAX_CHIPS		1

#define NAND_DISABLE_CE(nand)	(NFCONT_REG |= (1 << 1))
#define NAND_ENABLE_CE(nand)	(NFCONT_REG &= ~(1 << 1))
#define NF_TRANSRnB()		do { while(!(NFSTAT_REG & (1 << 0))); } while(0)

#define CFG_NAND_SKIP_BAD_DOT_I		1  /* ".i" read skips bad blocks   */
#define	CFG_NAND_WP			1
#define CFG_NAND_YAFFS_WRITE		1  /* support yaffs write */

#define CFG_ENV_IS_IN_NAND
#define CFG_ENV_OFFSET 0x0003C000

/* MMC configuration */
#define CONFIG_MMC		1
#define CFG_MMC_BASE		(0xf0000000)
#define CFG_MAX_MMC_DEVICE	1

/* OneNAND configuration */
#define CONFIG_ONENAND
#define CFG_ONENAND_BASE	(0x08000000)
#define CFG_MAX_ONENAND_DEVICE	1
#define CONFIG_REMOVE_OLD_ONENAND_CMD

#endif	/* __CONFIG_H */
