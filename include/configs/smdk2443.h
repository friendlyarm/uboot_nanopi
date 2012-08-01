/*
 * (C) Copyright 2005-2006
 * Seung-Chull, Suh <sc.suh@samsung.com>
 *
 * Configuation settings for the SAMSUNG SMDK2443 board.
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
#define	CONFIG_S3C2443		1		/* in a SAMSUNG S3C2443 SoC     */
#define	CONFIG_S3C24XX		1		/* in a SAMSUNG S3C24XX Family  */
#define CONFIG_SMDK2443		1		/* on a SAMSUNG SMDK2443 Board  */

/* input clock of PLL */
#define CONFIG_SYS_CLK_FREQ	12000000	/* the SMDK2443 has 12MHz input clock */

#define CONFIG_ENABLE_MMU
#ifdef CONFIG_ENABLE_MMU
#define virt_to_phys(x)	virt_to_phy_smdk2443(x)
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
#define MACH_TYPE		1084
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
#define CS8900_BASE		(0x09000300)
#define CS8900_BUS16		1 /* the Linux driver does accesses as shorts */

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1		1	/* we use SERIAL 1 on SMDK2443 */

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
			CFG_CMD_USB	| \
			CFG_CMD_REGINFO	| \
			CFG_CMD_LOADS	| \
			CFG_CMD_LOADB	| \
			CFG_CMD_DATE	| \
			CFG_CMD_JFFS2	| \
			CFG_CMD_PING	| \
			CFG_CMD_RUN	| \
			CFG_CMD_MMC	| \
			CFG_CMD_FAT	| \
			CFG_CMD_ELF)	\
			& ~(CFG_CMD_AUTOSCRIPT	| \
				CFG_CMD_BOOTD	| \
				CFG_CMD_IMI	| \
				CFG_CMD_IMLS	| \
				CFG_CMD_CONSOLE	| \
				0)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"u-boot=scsuh/u-boot.bin\0"					\
	"bootfile=scsuh/zImage\0"					\
	"rootpath=/opt/eldk4.0/arm\0"					\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath} "			\
		"console=ttyS0,${baudrate}\0"				\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"flash_self=run ramargs addip;"					\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"flash_nfs=run nfsargs addip;"					\
		"bootm ${kernel_addr}\0"				\
	"net_nfs=tftp c0008000 ${bootfile};run nfsargs addip;bootm\0"	\
	"load=tftp c0000000 ${u-boot}\0"					\
	"rmenv=nand erase 3c000 4000\0"					\
	"update=tftp c0000000 ${u-boot};"				\
		"nand erase 0 ${filesize};"				\
		"nand write c0000000 0 ${filesize};"


#define CONFIG_BOOTDELAY	3
/*#define CONFIG_BOOTARGS    	"root=ramfs devfs=mount console=ttySA0,9600" */
#define CONFIG_ETHADDR		00:40:5c:26:0a:5b
#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_IPADDR		192.168.0.20
#define CONFIG_SERVERIP		192.168.0.10
#define CONFIG_GATEWAYIP	192.168.0.1

#define CONFIG_ZERO_BOOTDELAY_CHECK
#define CONFIG_BOOTCOMMAND	"nand read c0008000 40000 1c0000;bootm c0008000"

/*#define CONFIG_NET_MULTI	1 */

#define CONFIG_MMC		1
#define CFG_MMC_BASE		0xf0000000

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200		/* speed to run kgdb serial port */
/* what's this ? it's not used anywhere */
#define CONFIG_KGDB_SER_INDEX	1		/* which serial port to use */
#endif

#define CONFIG_JFFS2_NAND	1	/* jffs2 on nand support */
#define NAND_CACHE_PAGES	16	/* size of nand cache in 512 bytes pages */

/*
 * JFFS2 partitions
 */
#undef CONFIG_JFFS2_CMDLINE
#define CONFIG_JFFS2_DEV		"nand0"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00200000

/* mtdparts command line support */
/*
#define CONFIG_JFFS2_CMDLINE
#define MTDIDS_DEFAULT		"nor0=r360-0"
#define MTDPARTS_DEFAULT	"mtdparts=r360-0:-@3m(user)"
*/

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory		*/
#define	CFG_PROMPT		"SMDK2443 # "	/* Monitor Command Prompt	*/
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
 * Clock Configuration
 */

#define CONFIG_CLK_533_133_66
//#define CONFIG_CLK_400_100_50

#if defined(CONFIG_CLK_400_100_50)
#define	Startup_MDIV		92
#define	Startup_PDIV		3
#define	Startup_SDIV		1
#define	Startup_ARMCLKdiv	0
#define	Startup_PREdiv		0x1
#define	Startup_HCLKdiv		0x1
#define	Startup_PCLKdiv		1
#elif defined (CONFIG_CLK_533_133_66)
#define	Startup_MDIV		81
#define	Startup_PDIV		2
#define	Startup_SDIV		1
#define	Startup_ARMCLKdiv	0
#define	Startup_PREdiv		0x1
#define	Startup_HCLKdiv		0x1
#define	Startup_PCLKdiv		1
#elif defined (CONFIG_CLK_533_89_44)
#define	Startup_MDIV		81
#define	Startup_PDIV		2
#define	Startup_SDIV		1
#define	Startup_ARMCLKdiv	0
#define	Startup_PREdiv		0x2
#define	Startup_HCLKdiv		0x1
#define	Startup_PCLKdiv		1
#endif

#define CLK_DIV_VAL	((Startup_ARMCLKdiv<<9)|(Startup_PREdiv<<4)|(Startup_PCLKdiv<<2)|(Startup_HCLKdiv)|(1<<3))
#define MPLL_VAL	((0<<24)|(Startup_MDIV<<16)|(Startup_PDIV<<8)|(Startup_SDIV))

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */

#define CONFIG_SDRAM_AUTO_DETECT

#ifdef CONFIG_SDRAM_AUTO_DETECT
  #define	CFG_BANK_CFG_VAL	0x0004920d
  #define	CFG_BANK_CON1_VAL	0x04000040

  #define	CFG_BANK_CFG_VAL_ALT	0x00048904
  #define	CFG_BANK_CON1_VAL_ALT	0x04000040
#else
  #ifdef CONFIG_SDR_CONFIG /* SDR */
    #define	CFG_BANK_CFG_VAL	0x00048904
    #define	CFG_BANK_CON1_VAL	0x04000040
  #else	/* DDR */
    #define	CFG_BANK_CFG_VAL	0x0004920d
    #define	CFG_BANK_CON1_VAL	0x04000040
  #endif
#endif

#if defined(CONFIG_CLK_533_133_66) || defined(CONFIG_CLK_533_89_44)
/* bankcon2 is over-clocked. by scsuh */
#define	CFG_BANK_CON2_VAL	0x0057003a /* org: 0x0069003f */
#define	CFG_BANK_CON3_VAL	0x80000030
#define	CFG_BANK_REFRESH_VAL	0x0000041d
#else	/* when HCLK is 100 */
#define	CFG_BANK_CON2_VAL	0x0057003a
#define	CFG_BANK_CON3_VAL	0x80000030
#define	CFG_BANK_REFRESH_VAL	0x00000313
#endif

#define CONFIG_NR_DRAM_BANKS	1	   /* we have 2 bank of DRAM */
#define PHYS_SDRAM_1		0x30000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x04000000 /* 64 MB */

#define CFG_FLASH_BASE		0x08000000

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
 * SMDK2443 board specific data
 */

#define CONFIG_IDENT_STRING	" for SMDK2443"

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

#define NAND_DISABLE_CE(nand)		(NFCONT_REG |= (1 << 1))
#define NAND_ENABLE_CE(nand)		(NFCONT_REG &= ~(1 << 1))
#define NF_TRANSRnB()		do { while(!(NFSTAT_REG & (1 << 0))); } while(0)

#define CFG_NAND_SKIP_BAD_DOT_I		1  /* ".i" read skips bad blocks   */
#define	CFG_NAND_WP			1
#define CFG_NAND_YAFFS_WRITE		1  /* support yaffs write */

#define CFG_ENV_IS_IN_NAND
#define CFG_ENV_OFFSET 0x0003C000
#define CFG_NAND_HWECC
//#define CFG_NAND_FLASH_BBT
#define CFG_NAND_LARGEPAGE_SAVEENV

#endif	/* __CONFIG_H */
