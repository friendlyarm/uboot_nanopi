/*
 * (C) Copyright 2006
 * Samsung Electronics.
 * Seung-chull, Suh <sc.suh@samsung.com>
 *
 * Configuation settings for the SAMSUNG SMDK2442 board.
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

/*
 * smdk2442 is based on smdk2440 because s3c2442 is derived from s3c2440.
 * So most code is shared with 2440.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define	CONFIG_S3C2442	1		/* in a SAMSUNG S3C2442 SoC     */
#define	CONFIG_S3C24XX	1		/* in a SAMSUNG S3C24XX Family  */
#define CONFIG_SMDK2442	1		/* on a SAMSUNG SMDK2442 Board */
//#define CONFIG_SP3000	1		/* on a Wingtech SP3000 Board */

/* input clock of PLL */
#define CONFIG_SYS_CLK_FREQ	16934400

#define CONFIG_ENABLE_MMU
#ifdef CONFIG_ENABLE_MMU
#define virt_to_phys(x)	virt_to_phy_smdk2440(x)
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
#define MACH_TYPE		1008
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
#if defined(CONFIG_SMDK2442)
#define CONFIG_DRIVER_CS8900	1	/* we have a CS8900 on-board */
#elif defined(CONFIG_SP3000)
#define CONFIG_DRIVER_CS8900	0
#else
#define CONFIG_DRIVER_CS8900	0
#endif

#define CS8900_BASE		0x19000300
#define CS8900_BUS16		1 /* the Linux driver does accesses as shorts */

/*
 * select serial console configuration
 */
#if defined(CONFIG_SMDK2442)
#define CONFIG_SERIAL1		/* we use UART0 on SMDK2442 */
#elif defined(CONFIG_SP3000)
#define CONFIG_SERIAL3          /* SP3000 uses the UART2 as console */
#endif

#define	CFG_HUSH_PARSER			/* use "hush" command parser	*/
#ifdef	CFG_HUSH_PARSER
#define	CFG_PROMPT_HUSH_PS2	"> "
#endif

#define CONFIG_CMDLINE_EDITING

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
#define CONFIG_S3C_USBD

#define USBD_DOWN_ADDR		0xc0000000

#define USBD_DETECT_IRQ()	(SRCPND_REG & (1<<25))
#define USBD_CLEAR_IRQ()	do { SRCPND_REG = (1<<25); \
					INTPND_REG= (1<<25); \
				} while (0)

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
			CFG_CMD_USB	| \
			CFG_CMD_REGINFO	| \
			CFG_CMD_LOADS	| \
			CFG_CMD_LOADB	| \
			CFG_CMD_DATE	| \
			CFG_CMD_JFFS2	| \
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

#if defined(CONFIG_SMDK2442)
#define CFG_PROMPT		"SMDK2442 # "	/* Monitor Command Prompt	*/
#elif defined(CONFIG_SP3000)
#define	CFG_PROMPT		"SP3000 # "	/* Monitor Command Prompt	*/
#else
#define CFG_PROMPT		"S3C2442 # "
#endif

#define	CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CFG_PBSIZE		384		/* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#undef  CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

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
 * Virtual/Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	2	   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x30000000 /* Bank #6 */
#define PHYS_SDRAM_1_SIZE	0x02000000 /* 32 MB */
#define PHYS_SDRAM_2		0x32000000 /* Bank #7 */
#define PHYS_SDRAM_2_SIZE	0x02000000 /* 32 MB */

#define VIRT_SDRAM_START_ADDR	0xc0000000
#define PHYS_SDRAM_SIZE		(PHYS_SDRAM_1_SIZE+PHYS_SDRAM_2_SIZE)

/* total memory required by uboot */
#define CFG_UBOOT_SIZE		(2*1024*1024)

/* base address for uboot */
#define CFG_UBOOT_BASE		0xc3e00000
#define CFG_PHY_UBOOT_BASE	0x33e00000

#ifdef CONFIG_ENABLE_MMU
#define CFG_MEMTEST_START	VIRT_SDRAM_START_ADDR
#define CFG_MEMTEST_END		CFG_UBOOT_BASE
#else
#define CFG_MEMTEST_START	PHYS_SDRAM_1
#define CFG_MEMTEST_END		CFG_PHY_UBOOT_BASE
#endif

/* default load address	*/
#define	CFG_LOAD_ADDR		VIRT_SDRAM_START_ADDR

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
 * SMDK2442 board specific data
 */

#if defined (CONFIG_SMDK2442)
#define CONFIG_IDENT_STRING	" for SMDK2442"
#elif defined (CONFIG_SP3000)
#define CONFIG_IDENT_STRING	" for SP3000"
#else
#define CONFIG_IDENT_STRING	""
#endif

/* NAND configuration */
#define CFG_MAX_NAND_DEVICE	1

#define CFG_NAND_BASE		(0x4e000010)
#define NAND_MAX_CHIPS		1

#define NAND_DISABLE_CE(nand)		(NFCONT_REG |= (1 << 1))
#define NAND_ENABLE_CE(nand)		(NFCONT_REG &= ~(1 << 1))
#define NF_TRANSRnB()			do { while(!(NFSTAT_REG & (1 << 0))); } while(0)

#define CFG_NAND_SKIP_BAD_DOT_I		1  /* ".i" read skips bad blocks   */
#define	CFG_NAND_WP			1
#define CFG_NAND_YAFFS_WRITE		1  /* support yaffs write */

#define CFG_ENV_IS_IN_NAND
#define CFG_ENV_OFFSET 0x0003C000
#define CFG_NAND_HWECC
//#define CFG_NAND_FLASH_BBT
#define CFG_NAND_LARGEPAGE_SAVEENV

#endif	/* __CONFIG_H */
