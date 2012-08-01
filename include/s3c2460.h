/*
 * (C) Copyright 2003
 * David Müller ELSOFT AG Switzerland. d.mueller@elsoft.ch
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

/************************************************
 * NAME	    : s3c2460.h
 *
 * Based on S3C2460X User's manual Rev 1.1
 ************************************************/

#ifndef __S3C2460_H__
#define __S3C2460_H__

#ifndef CONFIG_S3C2460
#define CONFIG_S3C2460		1
#endif

#define S3C24X0_UART_CHANNELS	2
#define S3C24X0_SPI_CHANNELS	2

#include <asm/hardware.h>

#ifndef __ASSEMBLY__
typedef enum {
	S3C24X0_UART0,
	S3C24X0_UART1,
} S3C24X0_UARTS_NR;

#include <s3c24x0.h>
#endif

#define ROM_BASE0		0x00000000      /* base address of rom bank 0 */
#define ROM_BASE1		0x04000000      /* base address of rom bank 1 */
#define DRAM_BASE0		0x10000000      /* base address of dram bank 0 */
#define DRAM_BASE1		0x20000000      /* base address of dram bank 1 */

/* S3C2460 only supports 512 Byte HW ECC */
#define S3C2410_ECCSIZE		512
#define S3C2410_ECCBYTES	3

/* S3C2460 device base addresses */
#define ELFIN_DMA_BASE		0x40400000
#define ELFIN_USB_HOST_BASE	0x41000000
#define ELFIN_LCD_BASE		0x4a000000
#define ELFIN_USB_DEVICE_BASE	0x44a00000
#define ELFIN_I2C_BASE		0x54000000
#define ELFIN_I2S_BASE		0x55000000
#define ELFIN_ADC_BASE		0x58000000
#define ELFIN_SPI_BASE		0x59000000
#define ELFIN_SDI_BASE		0x5A000000

/*
 * Clock and power management
 */

#define ELFIN_CLOCK_POWER_BASE	0x40000000

/* Clock & Power Controller */
#define LOCKCON0_OFFSET		0x00
#define LOCKCON1_OFFSET		0x04
#define OSCSET_OFFSET		0x08
#define MPLLCON_OFFSET		0x10
#define UPLLCON_OFFSET		0x14
#define EPLLCON_OFFSET		0x18
#define CLKSRCCON_OFFSET	0x20
#define CLKDIVCON_OFFSET	0x24
#define CLKOUTCON_OFFSET	0x28
#define HCLKCON_OFFSET		0x30
#define PCLKCON_OFFSET		0x34
#define SCLKCON_OFFSET		0x38
#define PWRMODECON_OFFSET	0x40
#define SWRSTCON_OFFSET		0x44
#define M_D_BUSPRI_OFFSET	0x50
#define F_I_BUSPRI_OFFSET	0x54
#define BUS_MISC_OFFSET		0x58
#define SYSID_OFFSET		0x5c
#define PWRCFG_OFFSET		0x60
#define RSTCNT_OFFSET		0x64
#define RSTSTAT_OFFSET		0x68
#define WKUPSTAT_OFFSET		0x6c
#define INFORM0_OFFSET		0x70
#define INFORM1_OFFSET		0x74
#define INFORM2_OFFSET		0x78
#define INFORM3_OFFSET		0x7c

#define LOCKCON0_REG		__REG(0x40000000)
#define LOCKCON1_REG		__REG(0x40000004)
#define OSCSET_REG		__REG(0x40000008)
#define MPLLCON_REG		__REG(0x40000010)
#define UPLLCON_REG		__REG(0x40000014)
#define EPLLCON_REG		__REG(0x40000018)
#define CLKSRC_REG		__REG(0x40000020)
#define CLKDIV_REG		__REG(0x40000024)
#define CLKOUTCON_REG		__REG(0x40000028)
#define HCLKCON_REG		__REG(0x40000030)
#define PCLKCON_REG		__REG(0x40000034)
#define SCLKCON_REG		__REG(0x40000038)
#define PWRMODECON_REG		__REG(0x40000040)
#define SWRSTCON_REG		__REG(0x40000044)
#define M_D_BUSPRI_REG		__REG(0x40000050)
#define F_I_BUSPRI_REG		__REG(0x40000054)
#define BUS_MISC_REG		__REG(0x40000058)
#define SYSID_REG		__REG(0x4000005c)
#define PWRCFG_REG		__REG(0x40000060)
#define RSTCNT_REG		__REG(0x40000064)
#define RSTSTAT_REG		__REG(0x40000068)
#define WKUPSTAT_REG		__REG(0x4000006c)
#define INFORM0_REG		__REG(0x40000070)
#define INFORM1_REG		__REG(0x40000074)
#define INFORM2_REG		__REG(0x40000078)
#define INFORM3_REG		__REG(0x4000007c)

/* for 0x20 CLKSRC */
#define I2S_CLK_SRC		0x00008000
#define CSI_CLK_SRC		0x00004000
#define CAM_CLK_SRC		0x00002000
#define FORTY8_CLK_SRC		0x00001000
#define I2S_CLK_DIV_SRC		0x00000800
#define UART_CLK_DIV_SRC	0x00000400
#define SelEPLL			0x00000040
#define SelUPLL			0x00000020
#define SelMPLL			0x00000010
#define SelExtDiv		0x00000008


/*
 * Bus Matrix
 */

#define ELFIN_BUS_SPE_BASE	0x40ce0000

#define PRIORITY_PORT0_OFFSET	0x00
#define PRIORITY_PORT1_OFFSET	0x04
#define PRIORITY_PORT2_OFFSET	0x08

/*
 * GPIO
 */

#define ELFIN_GPIO_BASE	0x44800000

#define GPACON_OFFSET		0x00
#define GPADAT_OFFSET		0x04
#define GPAPU_OFFSET		0x08
#define GPASLPCON_OFFSET	0x0C
#define GPABON_OFFSET		0x10
#define GPBDAT_OFFSET		0x14
#define GPBPU_OFFSET		0x18
#define GPBSLPCON_OFFSET	0x1C
#define GPCCON_OFFSET		0x20
#define GPCDAT_OFFSET		0x24
#define GPCPU_OFFSET		0x28
#define GPCSLPCON_OFFSET	0x2C
#define GPDCON_OFFSET		0x30
#define GPDDAT_OFFSET		0x34
#define GPDPU_OFFSET		0x38
#define GPECON_OFFSET		0x40
#define GPEDAT_OFFSET		0x44
#define GPEPU_OFFSET		0x48
#define GPESLPCON_OFFSET	0x4C
#define GPFCON_OFFSET		0x50
#define GPFDAT_OFFSET		0x54
#define GPFPU_OFFSET		0x58
#define GPFSLPCON_OFFSET	0x5C
#define GPGCON_OFFSET		0x60
#define GPGDAT_OFFSET		0x64
#define GPGPU_OFFSET		0x68
#define GPGSLPCON_OFFSET	0x6C
#define GPHCON_OFFSET		0x70
#define GPHDAT_OFFSET		0x74
#define GPHPU_OFFSET		0x78
#define GPHSLPCON_OFFSET	0x7C
#define GPICON_OFFSET		0x80
#define GPIDAT_OFFSET		0x84
#define GPIPU_OFFSET		0x88
#define GPISLPCON_OFFSET	0x8C
#define GPJCON_OFFSET		0x90
#define GPJDAT_OFFSET		0x94
#define GPJPU_OFFSET		0x98
#define SPCON_OFFSET		0x9C
#define GPKCON_OFFSET		0xA0
#define GPKDAT_OFFSET		0xA4
#define GPKPU_OFFSET		0xA8
#define GPKSLPCON_OFFSET	0xAC
#define EINTCON0_OFFSET		0xB0
#define EINTCON1_OFFSET		0xB4
#define EINTFLT0_OFFSET		0xB8
#define EINTFLT1_OFFSET		0xBC
#define EINTMASK_OFFSET		0xC0
#define EINTPEND_OFFSET		0xC4
#define DSPGPSLP_OFFSET		0xC8
#define MEM0CONSTOP_OFFSET	0xD0
#define MEM1CONSTOP_OFFSET	0xD4
#define MEM2CONSTOP_OFFSET	0xD8
#define MEM0CONSLP_OFFSET	0xE0
#define MEM1CONSLP_OFFSET	0xE4
#define MEM2CONSLP_OFFSET	0xE8
#define SPCONSLP_OFFSET		0xEC
#define SLPEN_OFFSET		0xF0
#define MEM0DRVCON_OFFSET	0xF4
#define MEM1DRVCON_OFFSET	0xF8
#define MEM2DRVCON_OFFSET	0xFC

#define GPACON_REG		__REG(0x44800000)
#define GPADAT_REG		__REG(0x44800004)
#define GPAPU_REG		__REG(0x44800008)
#define GPASLPCON_REG		__REG(0x4480000C)
#define GPABON_REG		__REG(0x44800010)
#define GPBDAT_REG		__REG(0x44800014)
#define GPBPU_REG		__REG(0x44800018)
#define GPBSLPCON_REG		__REG(0x4480001C)
#define GPCCON_REG		__REG(0x44800020)
#define GPCDAT_REG		__REG(0x44800024)
#define GPCPU_REG		__REG(0x44800028)
#define GPCSLPCON_REG		__REG(0x4480002C)
#define GPDCON_REG		__REG(0x44800030)
#define GPDDAT_REG		__REG(0x44800034)
#define GPDPU_REG		__REG(0x44800038)
#define GPECON_REG		__REG(0x44800040)
#define GPEDAT_REG		__REG(0x44800044)
#define GPEPU_REG		__REG(0x44800048)
#define GPESLPCON_REG		__REG(0x4480004C)
#define GPFCON_REG		__REG(0x44800050)
#define GPFDAT_REG		__REG(0x44800054)
#define GPFPU_REG		__REG(0x44800058)
#define GPFSLPCON_REG		__REG(0x4480005C)
#define GPGCON_REG		__REG(0x44800060)
#define GPGDAT_REG		__REG(0x44800064)
#define GPGPU_REG		__REG(0x44800068)
#define GPGSLPCON_REG		__REG(0x4480006C)
#define GPHCON_REG		__REG(0x44800070)
#define GPHDAT_REG		__REG(0x44800074)
#define GPHPU_REG		__REG(0x44800078)
#define GPHSLPCON_REG		__REG(0x4480007C)
#define GPICON_REG		__REG(0x44800080)
#define GPIDAT_REG		__REG(0x44800084)
#define GPIPU_REG		__REG(0x44800088)
#define GPISLPCON_REG		__REG(0x4480008C)
#define GPJCON_REG		__REG(0x44800090)
#define GPJDAT_REG		__REG(0x44800094)
#define GPJPU_REG		__REG(0x44800098)
#define SPCON_REG		__REG(0x4480009C)
#define GPKCON_REG		__REG(0x448000A0)
#define GPKDAT_REG		__REG(0x448000A4)
#define GPKPU_REG		__REG(0x448000A8)
#define GPKSLPCON_REG		__REG(0x448000AC)
#define EINTCON0_REG		__REG(0x448000B0)
#define EINTCON1_REG		__REG(0x448000B4)
#define EINTFLT0_REG		__REG(0x448000B8)
#define EINTFLT1_REG		__REG(0x448000BC)
#define EINTMASK_REG		__REG(0x448000C0)
#define EINTPEND_REG		__REG(0x448000C4)
#define DSPGPSLP_REG		__REG(0x448000C8)
#define MEM0CONSTOP_REG		__REG(0x448000D0)
#define MEM1CONSTOP_REG		__REG(0x448000D4)
#define MEM2CONSTOP_REG		__REG(0x448000D8)
#define MEM0CONSLP_REG		__REG(0x448000E0)
#define MEM1CONSLP_REG		__REG(0x448000E4)
#define MEM2CONSLP_REG		__REG(0x448000E8)
#define SPCONSLP_REG		__REG(0x448000EC)
#define SLPEN_REG		__REG(0x448000F0)
#define MEM0DRVCON_REG		__REG(0x448000F4)
#define MEM1DRVCON_REG		__REG(0x448000F8)
#define MEM2DRVCON_REG		__REG(0x448000FC)


/*
 * Memory controller
 */

#define ELFIN_SROM_BASE	0x40c20000

#define SROM_BW_REG		__REG(0x40C20000)
#define SROM_BC0_REG		__REG(0x40C20004)
#define SROM_BC1_REG		__REG(0x40C20008)
#define SROM_BC2_REG		__REG(0x40C2000C)

/*
 * SDRAM Controller
 */

#define ELFIN_MEMCTL0_BASE	0x40c40000
#define ELFIN_MEMCTL1_BASE	0x40c80000
#define SDRAM_BANKCFG_OFFSET	0x00
#define SDRAM_BANKCON_OFFSET	0x04
#define SDRAM_REFRESH_OFFSET	0x08
#define SDRAM_TIMEOUT_OFFSET	0x0c

#define INIT_NORMAL	0x0
#define INIT_PALL	0x1
#define INIT_MRS	0x2
#define INIT_EMRS	0x3
#define INIT_MASK	0x3


/*
 * Nand flash controller
 */

#define ELFIN_NAND_BASE	0x40c00000

#define NFCONF_OFFSET		0x00
#define NFCONT_OFFSET		0x04
#define NFCMMD_OFFSET		0x08
#define NFADDR_OFFSET		0x0c
#define NFDATA_OFFSET		0x10
#define NFMECCDATA0_OFFSET	0x14
#define NFMECCDATA1_OFFSET	0x18
#define NFMECCDATA2_OFFSET	0x1c
#define NFMECCDATA3_OFFSET	0x20
#define NFSECCDATA0_OFFSET	0x24
#define NFSECCDATA1_OFFSET	0x28
#define NFSTAT_OFFSET		0x2c
#define NFESTAT0_OFFSET		0x30
#define NFESTAT1_OFFSET		0x34
#define NFMECC0_OFFSET		0x38
#define NFMECC1_OFFSET		0x3c
#define NFSECC_OFFSET		0x40
#define NFSBLK_OFFSET		0x44
#define NFEBLK_OFFSET		0x48

#define NFCONF			(0x40C00000)
#define NFCONT			(0x40C00004)
#define NFCMMD			(0x40C00008)
#define NFADDR			(0x40C0000C)
#define NFDATA			(0x40C00010)
#define NANDECCD0		(0x40C00014)
#define NANDECCD1		(0x40C00018)
#define NANDECCD2		(0x40C0001C)
#define NANDECCD3		(0x40C00020)
#define NANDOOBECC0		(0x40C00024)
#define NANDOOBECC1		(0x40C00028)
#define NANDSTAT		(0x40C0002C)
#define NFSTAT			(0x40C0002C)
#define NANDESTAT0		(0x40C00030)
#define NANDESTAT1		(0x40C00034)
#define NANDECC0		(0x40C00038)
#define NANDECC1		(0x40C0003C)
#define NANDOOBECC		(0x40C00040)


#define NFCONF_REG		__REG(0x40C00000)
#define NFCONT_REG		__REG(0x40C00004)
#define NFCMD_REG		__REGb(0x40C00008)
#define NFADDR_REG		__REGb(0x40C0000C)
#define NFDATA8_REG		__REGb(0x40C00010)
#define NFDATA16_REG		__REGw(0x40C00010)
#define NFDATA32_REG		__REG(0x40C00010)
#define NANDECCD0_REG		__REG(0x40C00014)
#define NANDECCD1_REG		__REG(0x40C00018)
#define NANDECCD2_REG		__REG(0x40C0001C)
#define NANDECCD3_REG		__REG(0x40C00020)
#define NANDOOBECC0_REG		__REG(0x40C00024)
#define NANDOOBECC1_REG		__REG(0x40C00028)
#define NANDSTAT_REG		__REG(0x40C0002C)
#define NFSTAT_REG		__REG(0x40C0002C)
#define NANDESTAT0_REG		__REG(0x40C00030)
#define NANDESTAT1_REG		__REG(0x40C00034)
#define NANDECC0_REG		__REG(0x40C00038)
#define NANDECC1_REG		__REG(0x40C0003C)
#define NANDOOBECC_REG		__REG(0x40C00040)

#define NFCONT_CS		(1<<7)
#define NFCONT_CS_ALT		(1<<7)
#define NFSTAT_RnB		(1<<10)
#define NFCONT_WP		(1<<2)

/*
 * Interrupt
 */
#define ELFIN_INTERRUPT_BASE	0x40200000

#define INTMSK_OFFSET		0x08
#define INTSUBMSK_OFFSET	0x1c
#define INTMOD_OFFSET		0x04

#define SRCPND_REG		__REG(0x40200000)
#define INTMOD_REG		__REG(0x40200004)
#define INTMSK_REG		__REG(0x40200008)
#define INTPND_REG		__REG(0x40200010)
#define INTOFFSET_REG		__REG(0x40200014)
#define SUBSRCPND_REG		__REG(0x40200018)
#define INTSUBMSK_REG		__REG(0x4020001C)
#define SUBSRCPND2_REG		__REG(0x40200030)
#define INTSUBMSK2_REG		__REG(0x40200034)
#define PRIORITY_MODE_REG	__REG(0x40200038)
#define PRIORITY_UPDATE_REG	__REG(0x4020003c)


/*
 * Watchdog timer
 */

#define ELFIN_WATCHDOG_BASE		0x44100000

#define WTCON_REG		__REG(0x44100000)
#define WTDAT_REG		__REG(0x44100004)
#define WTCNT_REG		__REG(0x44100008)


/*
 * UART
 */
#define ELFIN_UART_BASE	0x44400000

#define ULCON0_REG		__REG(0x44400000)
#define UCON0_REG		__REG(0x44400004)
#define UFCON0_REG		__REG(0x44400008)
#define UMCON0_REG		__REG(0x4440000C)
#define UTRSTAT0_REG		__REG(0x44400010)
#define UERSTAT0_REG		__REG(0x44400014)
#define UFSTAT0_REG		__REG(0x44400018)
#define UMSTAT0_REG		__REG(0x4440001C)
#define UTXH0_REG		__REG(0x44400020)
#define URXH0_REG		__REG(0x44400024)
#define UBRDIV0_REG		__REG(0x44400028)

#define ULCON1_REG		__REG(0x44404000)
#define UCON1_REG		__REG(0x44404004)
#define UFCON1_REG		__REG(0x44404008)
#define UMCON1_REG		__REG(0x4440400C)
#define UTRSTAT1_REG		__REG(0x44404010)
#define UERSTAT1_REG		__REG(0x44404014)
#define UFSTAT1_REG		__REG(0x44404018)
#define UMSTAT1_REG		__REG(0x4440401C)
#define UTXH1_REG		__REG(0x44404020)
#define URXH1_REG		__REG(0x44404024)
#define UBRDIV1_REG		__REG(0x44404028)

#define UTRSTAT_TX_EMPTY	(1 << 2)
#define UTRSTAT_RX_READY	(1 << 0)
#define UART_ERR_MASK		0xF

/*
 * PWM timer
 */
#define ELFIN_TIMER_BASE	0x44000000

#define TCFG0_REG		__REG(0x44000000)
#define TCFG1_REG		__REG(0x44000004)
#define TCON_REG		__REG(0x44000008)
#define TCNTB0_REG		__REG(0x4400000C)
#define TCMPB0_REG		__REG(0x44000010)
#define TCNTO0_REG		__REG(0x44000014)
#define TCNTB1_REG		__REG(0x44000018)
#define TCMPB1_REG		__REG(0x4400001C)
#define TCNTO1_REG		__REG(0x44000020)
#define TCNTB2_REG		__REG(0x44000024)
#define TCMPB2_REG		__REG(0x44000028)
#define TCNTO2_REG		__REG(0x4400002C)
#define TCNTB3_REG		__REG(0x44000030)
#define TCMPB3_REG		__REG(0x44000034)
#define TCNTO3_REG		__REG(0x44000038)
#define TCNTB4_REG		__REG(0x4400003C)
#define TCNTO4_REG		__REG(0x44000040)

/* Fields */
#define fTCFG0_DZONE		Fld(8,16)       /* the dead zone length (= timer 0) */
#define fTCFG0_PRE1		Fld(8,8)        /* prescaler value for time 2,3,4 */
#define fTCFG0_PRE0		Fld(8,0)        /* prescaler value for time 0,1 */
#define fTCFG1_MUX4		Fld(4,16)
/* bits */
#define TCFG0_DZONE(x)		FInsrt((x), fTCFG0_DZONE)
#define TCFG0_PRE1(x)		FInsrt((x), fTCFG0_PRE1)
#define TCFG0_PRE0(x)		FInsrt((x), fTCFG0_PRE0)
#define TCON_4_AUTO		(1 << 22)       /* auto reload on/off for Timer 4 */
#define TCON_4_UPDATE		(1 << 21)       /* manual Update TCNTB4 */
#define TCON_4_ONOFF		(1 << 20)       /* 0: Stop, 1: start Timer 4 */
#define COUNT_4_ON		(TCON_4_ONOFF*1)
#define COUNT_4_OFF		(TCON_4_ONOFF*0)
#define TCON_3_AUTO		(1 << 19)       /* auto reload on/off for Timer 3 */
#define TIMER3_ATLOAD_ON	(TCON_3_AUTO*1)
#define TIMER3_ATLAOD_OFF	FClrBit(TCON, TCON_3_AUTO)
#define TCON_3_INVERT		(1 << 18)       /* 1: Inverter on for TOUT3 */
#define TIMER3_IVT_ON		(TCON_3_INVERT*1)
#define TIMER3_IVT_OFF		(FClrBit(TCON, TCON_3_INVERT))
#define TCON_3_MAN		(1 << 17)       /* manual Update TCNTB3,TCMPB3 */
#define TIMER3_MANUP		(TCON_3_MAN*1)
#define TIMER3_NOP		(FClrBit(TCON, TCON_3_MAN))
#define TCON_3_ONOFF		(1 << 16)       /* 0: Stop, 1: start Timer 3 */
#define TIMER3_ON		(TCON_3_ONOFF*1)
#define TIMER3_OFF		(FClrBit(TCON, TCON_3_ONOFF))
/* macros */
#define GET_PRESCALE_TIMER4(x)	FExtr((x), fTCFG0_PRE1)
#define GET_DIVIDER_TIMER4(x)	FExtr((x), fTCFG1_MUX4)

/*
 * RTC Controller
 */
#define ELFIN_RTC_BASE	0x44200000
#define RTCCON_REG		__REG(0x44200040)
#define TICNT_REG		__REG(0x44200044)
#define RTCALM_REG		__REG(0x44200050)
#define ALMSEC_REG		__REG(0x44200054)
#define ALMMIN_REG		__REG(0x44200058)
#define ALMHOUR_REG		__REG(0x4420005c)
#define ALMDATE_REG		__REG(0x44200060)
#define ALMMON_REG		__REG(0x44200064)
#define ALMYEAR_REG		__REG(0x44200068)
#define BCDSEC_REG		__REG(0x44200070)
#define BCDMIN_REG		__REG(0x44200074)
#define BCDHOUR_REG		__REG(0x44200078)
#define BCDDATE_REG		__REG(0x4420007c)
#define BCDDAY_REG		__REG(0x44200080)
#define BCDMON_REG		__REG(0x44200084)
#define BCDYEAR_REG		__REG(0x44200088)

/* USB Device */
#define USB_DEVICE_PHYS_ADR	0x44a00000

/* include common stuff */
#ifndef __ASSEMBLY__
static inline S3C24X0_MEMCTL * S3C24X0_GetBase_MEMCTL(void)
{
	return (S3C24X0_MEMCTL *)(ELFIN_MEMCTL0_BASE);
}
static inline S3C24X0_USB_HOST * S3C24X0_GetBase_USB_HOST(void)
{
	return (S3C24X0_USB_HOST *)ELFIN_USB_HOST_BASE;
}
static inline S3C24X0_INTERRUPT * S3C24X0_GetBase_INTERRUPT(void)
{
	return (S3C24X0_INTERRUPT *)ELFIN_INTERRUPT_BASE;
}
static inline S3C24X0_DMAS * S3C24X0_GetBase_DMAS(void)
{
	return (S3C24X0_DMAS *)ELFIN_DMA_BASE;
}
static inline S3C24X0_CLOCK_POWER * S3C24X0_GetBase_CLOCK_POWER(void)
{
	return (S3C24X0_CLOCK_POWER *)ELFIN_CLOCK_POWER_BASE;
}
static inline S3C24X0_LCD * S3C24X0_GetBase_LCD(void)
{
	return (S3C24X0_LCD *)ELFIN_LCD_BASE;
}
static inline S3C2410_NAND * S3C2410_GetBase_NAND(void)
{
	return (S3C2410_NAND *)ELFIN_NAND_BASE;
}
static inline S3C24X0_UART * S3C24X0_GetBase_UART(S3C24X0_UARTS_NR nr)
{
	return (S3C24X0_UART *)(ELFIN_UART_BASE + (nr * 0x4000));
}
static inline S3C24X0_TIMERS * S3C24X0_GetBase_TIMERS(void)
{
	return (S3C24X0_TIMERS *)ELFIN_TIMER_BASE;
}
static inline S3C24X0_USB_DEVICE * S3C24X0_GetBase_USB_DEVICE(void)
{
	return (S3C24X0_USB_DEVICE *)ELFIN_USB_DEVICE_BASE;
}
static inline S3C24X0_WATCHDOG * S3C24X0_GetBase_WATCHDOG(void)
{
	return (S3C24X0_WATCHDOG *)ELFIN_WATCHDOG_BASE;
}
static inline S3C24X0_I2C * S3C24X0_GetBase_I2C(void)
{
	return (S3C24X0_I2C *)ELFIN_I2C_BASE;
}
static inline S3C24X0_I2S * S3C24X0_GetBase_I2S(void)
{
	return (S3C24X0_I2S *)ELFIN_I2S_BASE;
}
static inline S3C24X0_GPIO * S3C24X0_GetBase_GPIO(void)
{
	return (S3C24X0_GPIO *)ELFIN_GPIO_BASE;
}
static inline S3C24X0_RTC * S3C24X0_GetBase_RTC(void)
{
	return (S3C24X0_RTC *)ELFIN_RTC_BASE;
}
static inline S3C2410_ADC * S3C2410_GetBase_ADC(void)
{
	return (S3C2410_ADC *)ELFIN_ADC_BASE;
}
static inline S3C24X0_SPI * S3C24X0_GetBase_SPI(void)
{
	return (S3C24X0_SPI *)ELFIN_SPI_BASE;
}
static inline S3C2410_SDI * S3C2410_GetBase_SDI(void)
{
	return (S3C2410_SDI *)ELFIN_SDI_BASE;
}
#else /* #ifndef __ASSEMBLY__ */

/* watchdog */
#define WTCON_OFFSET		0x00

/* LCD controller */
#define LCDBGCON_OFFSET		0x5c

#endif /* #ifndef __ASSEMBLY__ */

/* PENDING BIT */
#define BIT_EINT0		(0x1)
#define BIT_EINT1		(0x1<<1)
#define BIT_EINT2		(0x1<<2)
#define BIT_EINT3		(0x1<<3)
#define BIT_EINT4_7		(0x1<<4)
#define BIT_EINT8_23		(0x1<<5)
#define BIT_BAT_FLT		(0x1<<7)
#define BIT_TICK		(0x1<<8)
#define BIT_WDT			(0x1<<9)
#define BIT_TIMER0		(0x1<<10)
#define BIT_TIMER1		(0x1<<11)
#define BIT_TIMER2		(0x1<<12)
#define BIT_TIMER3		(0x1<<13)
#define BIT_TIMER4		(0x1<<14)
#define BIT_UART2		(0x1<<15)
#define BIT_LCD			(0x1<<16)
#define BIT_DMA0		(0x1<<17)
#define BIT_DMA1		(0x1<<18)
#define BIT_DMA2		(0x1<<19)
#define BIT_DMA3		(0x1<<20)
#define BIT_SDI			(0x1<<21)
#define BIT_SPI0		(0x1<<22)
#define BIT_UART1		(0x1<<23)
#define BIT_USBH		(0x1<<26)
#define BIT_IIC			(0x1<<27)
#define BIT_UART0		(0x1<<28)
#define BIT_SPI1		(0x1<<29)
#define BIT_RTC			(0x1<<30)
#define BIT_ADC			(0x1<<31)
#define BIT_ALLMSK		(0xFFFFFFFF)

#define CLKCON_USBD			(1<< 15)
#define BIT19                           0x00080000
#define BIT29                           0x20000000
#define BIT_USBD			BIT29
#define BIT_USBD_SUB			BIT19

#define ClearPending(bit) {\
		 rSRCPND = bit;\
		 rINTPND = bit;\
		 rINTPND;\
		 }
/* Wait until rINTPND is changed for the case that the ISR is very short. */

#endif /*__S3C2460_H__*/
