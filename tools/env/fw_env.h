/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __UBOOT_FW_ENV_H__
#define __UBOOT_FW_ENV_H__		1

/*
 * To build the utility with the run-time configuration
 * uncomment the next line.
 * See included "fw_env.config" sample file (TRAB board)
 * for notes on configuration.
 */
#define CONFIG_FILE     "/etc/fw_env.config"

#ifdef CONFIG_BOOT_NAND
#define HAVE_REDUND /* For systems with 2 env sectors */
#define DEVICE1_NAME      "/dev/mtd1"
#define DEVICE2_NAME      "/dev/mtd2"
#define DEVICE1_OFFSET    0x0000
#define ENV1_SIZE         0x4000
#define DEVICE1_ESIZE     0x4000
#define DEVICE2_OFFSET    0x0000
#define ENV2_SIZE         0x4000
#define DEVICE2_ESIZE     0x4000
#endif

#ifdef CONFIG_BOOT_MOVINAND
#undef  CONFIG_FILE
#undef  HAVE_REDUND
#define DEVICE1_NAME      "/dev/mmcblk0"
#define DEVICE1_OFFSET    0x86400 /* SDHC: (1024+2+16+32)*512 */
#define ENV1_SIZE         0x4000
#define DEVICE1_ESIZE     0x4000  /* NOT used for SD/MMC */
#endif

#ifndef CONFIG_BAUDRATE
#define CONFIG_BAUDRATE   115200
#endif

#ifndef CONFIG_BOOTDELAY
#define CONFIG_BOOTDELAY  5 /* autoboot after 5 seconds */
#endif

#ifndef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND							\
	"bootp; " 								\
	"setenv bootargs root=/dev/nfs nfsroot=${serverip}:${rootpath} " 	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; " 	\
	"bootm"
#endif

extern void fw_printenv (int argc, char *argv[]);
extern unsigned char *fw_getenv (unsigned char *name);
extern int fw_setenv (int argc, char *argv[]);

extern unsigned long crc32 (unsigned long, const unsigned char *, unsigned);

#endif /* __UBOOT_FW_ENV_H__ */
