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
 * NAME	    : s3c2440.h
 *
 * Based on S3C2440A User's manual Rev 1.1
 ************************************************/

#ifndef __S3C2440_H__
#define __S3C2440_H__

#define S3C24X0_UART_CHANNELS	3
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
#define DRAM_BASE0		0x30000000      /* base address of dram bank 0 */
#define DRAM_BASE1		0x00000000      /* base address of dram bank 1 */

/* S3C2440 only supports 512 Byte HW ECC */
#define S3C2410_ECCSIZE		512
#define S3C2410_ECCBYTES	3

/* S3C2440 device base addresses */
#define ELFIN_DMA_BASE		0x4b000000
#define ELFIN_USB_DEVICE_BASE	0x44a00000
#define ELFIN_I2C_BASE		0x54000000
#define ELFIN_I2S_BASE		0x55000000
#define ELFIN_ADC_BASE		0x58000000
#define ELFIN_SPI_BASE		0x59000000
#define ELFIN_SDI_BASE		0x5A000000

/*
 * Memory controller
 */

#define ELFIN_MEM_CON_BASE	0x48000000

#define BWSCON_OFFSET		0x00
#define BANKCON0_OFFSET		0x04
#define BANKCON1_OFFSET		0x08
#define BANKCON2_OFFSET		0x0c
#define BANKCON3_OFFSET		0x10
#define BANKCON4_OFFSET		0x14
#define BANKCON5_OFFSET		0x18
#define BANKCON6_OFFSET		0x1c
#define BANKCON7_OFFSET		0x20
#define REFRESH_OFFSET		0x24
#define BANKSIZE_OFFSET		0x28
#define MRSRB6_OFFSET		0x2c
#define MRSRB7_OFFSET		0x30

#define BWSCON_REG		__REG(0x48000000)
#define BANKCON0_REG		__REG(0x48000004)
#define BANKCON1_REG		__REG(0x48000008)
#define BANKCON2_REG		__REG(0x4800000c)
#define BANKCON3_REG		__REG(0x48000010)
#define BANKCON4_REG		__REG(0x48000014)
#define BANKCON5_REG		__REG(0x48000018)
#define BANKCON6_REG		__REG(0x4800001c)
#define BANKCON7_REG		__REG(0x48000020)
#define REFRESH_REG		__REG(0x48000024)
#define BANKSIZE_REG		__REG(0x48000028)
#define MRSRB6_REG		__REG(0x4800002c)
#define MRSRB7_REG		__REG(0x48000030)


/*
 * USB Host Controller
 */
#define ELFIN_USB_HOST_BASE	0x49000000

#define HcRevision_OFFSET	0x00
#define HcControl_OFFSET	0x04
#define HcCommonStatus_OFFSET	0x08
#define HcIntStatus_OFFSET	0x0c
#define HcIntEnable_OFFSET	0x10
#define HcIntDisable_OFFSET	0x14
#define HcHCCA_OFFSET		0x18
#define HcPerdCurED_OFFSET	0x1c
#define HcCtrlHeadED_OFFSET	0x20
#define HcCtrlCurED_OFFSET	0x24
#define HcBulkHeadED_OFFSET	0x28
#define HcBulkCurED_OFFSET	0x2c
#define HcDoneHead_OFFSET	0x30
#define HcRmInterval_OFFSET	0x34
#define HcFmRemain_OFFSET	0x38
#define HcFmNumber_OFFSET	0x3c
#define HcPerdStart_OFFSET	0x40
#define HcLSThres_OFFSET	0x44
#define HcRhDescA_OFFSET	0x48
#define HcRhDescB_OFFSET	0x4c
#define HcRhStatus_OFFSET	0x50
#define HcRhPortSts1_OFFSET	0x54
#define HcRhPortSts2_OFFSET	0x58

#define HcRevision_REG		__REG(0x49000000)
#define HcControl_REG		__REG(0x49000004)
#define HcCommonStatus_REG	__REG(0x49000008)
#define HcIntStatus_REG		__REG(0x4900000c)
#define HcIntEnable_REG		__REG(0x49000010)
#define HcIntDisable_REG	__REG(0x49000014)
#define HcHCCA_REG		__REG(0x49000018)
#define HcPerdCurED_REG		__REG(0x4900001c)
#define HcCtrlHeadED_REG	__REG(0x49000020)
#define HcCtrlCurED_REG		__REG(0x49000024)
#define HcBulkHeadED_REG	__REG(0x49000028)
#define HcBulkCurED_REG		__REG(0x4900002c)
#define HcDoneHead_REG		__REG(0x49000030)
#define HcRmInterval_REG	__REG(0x49000034)
#define HcFmRemain_REG		__REG(0x49000038)
#define HcFmNumber_REG		__REG(0x4900003c)
#define HcPerdStart_REG		__REG(0x49000040)
#define HcLSThres_REG		__REG(0x49000044)
#define HcRhDescA_REG		__REG(0x49000048)
#define HcRhDescB_REG		__REG(0x4900004c)
#define HcRhStatus_REG		__REG(0x49000050)
#define HcRhPortSts1_REG	__REG(0x49000054)
#define HcRhPortSts2_REG	__REG(0x49000058)


/*
 * Interrupt
 */
#define ELFIN_INTERRUPT_BASE	0x4a000000

#define SRCPND_OFFSET		0x00
#define INTMOD_OFFSET		0x04
#define INTMSK_OFFSET		0x08
#define PRIORITY_OFFSET		0x0c
#define INTPND_OFFSET		0x10
#define INTOFFSET_OFFSET	0x14
#define SUBSRCPND_OFFSET	0x18
#define INTSUBMSK_OFFSET	0x1c

#define SRCPND_REG		__REG(0x4a000000)
#define INTMOD_REG		__REG(0x4a000004)
#define INTMSK_REG		__REG(0x4a000008)
#define PRIORITY_REG		__REG(0x4a00000c)
#define INTPND_REG		__REG(0x4a000010)
#define INTOFFSET_REG		__REG(0x4a000014)
#define SUBSRCPND_REG		__REG(0x4a000018)
#define INTSUBMSK_REG		__REG(0x4a00001C)


/*
 * Clock and power management
 */

#define ELFIN_CLOCK_POWER_BASE	0x4c000000

#define LOCKTIME_OFFSET		0x00
#define MPLLCON_OFFSET		0x04
#define UPLLCON_OFFSET		0x08
#define CLKCON_OFFSET		0x0c
#define CLKSLOW_OFFSET		0x10
#define CLKDIVN_OFFSET		0x14
#define CAMDIVN_OFFSET		0x18

#define LOCKTIME_REG		__REG(0x4c000000)
#define MPLLCON_REG		__REG(0x4c000004)
#define UPLLCON_REG		__REG(0x4c000008)
#define CLKCON_REG		__REG(0x4c00000c)
#define CLKSLOW_REG		__REG(0x4c000010)
#define CLKDIVN_REG		__REG(0x4c000014)
#define CAMDIVN_REG		__REG(0x4c000018)


/*
 * LCD Controller
 */
#define ELFIN_LCD_BASE		0x4d000000

/*
 * GPIO
 */

#define ELFIN_GPIO_BASE	0x56000000

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
 * Nand flash controller
 */

#define ELFIN_NAND_BASE	0x4e000000

#define NFCONF_OFFSET		0x00
#define NFCONT_OFFSET		0x04
#define NFCMMD_OFFSET		0x08
#define NFADDR_OFFSET		0x0c
#define NFDATA_OFFSET		0x10
#define NFMECCD0_OFFSET		0x14
#define NFMECCD1_OFFSET		0x18
#define NFSECCD_OFFSET		0x1c
#define NFSTAT_OFFSET		0x20
#define NFESTAT0_OFFSET		0x24
#define NFESTAT1_OFFSET		0x28
#define NFMECC0_OFFSET		0x2c
#define NFMECC1_OFFSET		0x30
#define NFSECC_OFFSET		0x34
#define NFSBLK_OFFSET		0x38
#define NFEBLK_OFFSET		0x3c

#define NFCONF		(0x4e000000)
#define NFCONT		(0x4e000004)
#define NFCMMD		(0x4e000008)
#define NFADDR		(0x4e00000C)
#define NFDATA		(0x4e000010)
#define NFMECCDATA0	(0x4e000014)
#define NFMECCDATA1	(0x4e000018)
#define NFSECCDATA	(0x4e00001c)
#define NFSTAT		(0x4e000020)
#define NFESTAT0	(0x4e000024)
#define NFESTAT1	(0x4e000028)
#define NFMECC0		(0x4e00002C)
#define NFMECC1		(0x4e000030)
#define NFSECC		(0x4e00003c)

#define NFCONF_REG		__REG(0x4e000000)
#define NFCONT_REG		__REG(0x4e000004)
#define NFCMD_REG		__REGb(0x4e000008)
#define NFADDR_REG		__REGb(0x4e00000C)
#define NFDATA8_REG		__REGb(0x4e000010)
#define NFDATA16_REG		__REGw(0x4e000010)
#define NFDATA32_REG		__REG(0x4e000010)
#define NFSTAT_REG		__REG(0x4e000020)

#define NFCONT_WP		(1<<12)
#define NFCONT_SECCLOCK		(1<<6)
#define NFCONT_MECCLOCK		(1<<5)
#define NFCONT_INITECC		(1<<4)
#define NFCONT_CS_ALT		(1<<1)
#define NFCONT_CS		(1<<1)
#define NFSTAT_RnB		(1<<0)

/*
 * Watchdog timer
 */

#define ELFIN_WATCHDOG_BASE	0x53000000

#define WTCON_OFFSET		0x00
#define WTDAT_OFFSET		0x04
#define WTCNT_OFFSET		0x08

#define WTCON_REG		__REG(0x53000000)
#define WTDAT_REG		__REG(0x53000004)
#define WTCNT_REG		__REG(0x53000008)


/*
 * UART
 */
#define ELFIN_UART_BASE		0x50000000

#define ULCON0_OFFSET		0x00
#define UCON0_OFFSET		0x04
#define UFCON0_OFFSET		0x08
#define UMCON0_OFFSET		0x0C
#define UTRSTAT0_OFFSET		0x10
#define UERSTAT0_OFFSET		0x14
#define UFSTAT0_OFFSET		0x18
#define UMSTAT0_OFFSET		0x1C
#define UTXH0_OFFSET		0x20
#define URXH0_OFFSET		0x24
#define UBRDIV0_OFFSET		0x28

#define ULCON0_REG		__REG(0x50000000)
#define UCON0_REG		__REG(0x50000004)
#define UFCON0_REG		__REG(0x50000008)
#define UMCON0_REG		__REG(0x5000000C)
#define UTRSTAT0_REG		__REG(0x50000010)
#define UERSTAT0_REG		__REG(0x50000014)
#define UFSTAT0_REG		__REG(0x50000018)
#define UMSTAT0_REG		__REG(0x5000001C)
#define UTXH0_REG		__REG(0x50000020)
#define URXH0_REG		__REG(0x50000024)
#define UBRDIV0_REG		__REG(0x50000028)

#define ULCON1_REG		__REG(0x50004000)
#define UCON1_REG		__REG(0x50004004)
#define UFCON1_REG		__REG(0x50004008)
#define UMCON1_REG		__REG(0x5000400C)
#define UTRSTAT1_REG		__REG(0x50004010)
#define UERSTAT1_REG		__REG(0x50004014)
#define UFSTAT1_REG		__REG(0x50004018)
#define UMSTAT1_REG		__REG(0x5000401C)
#define UTXH1_REG		__REG(0x50004020)
#define URXH1_REG		__REG(0x50004024)
#define UBRDIV1_REG		__REG(0x50004028)

#define ULCON2_REG		__REG(0x50008000)
#define UCON2_REG		__REG(0x50008004)
#define UFCON2_REG		__REG(0x50008008)
#define UMCON2_REG		__REG(0x5000800C)
#define UTRSTAT2_REG		__REG(0x50008010)
#define UERSTAT2_REG		__REG(0x50008014)
#define UFSTAT2_REG		__REG(0x50008018)
#define UMSTAT2_REG		__REG(0x5000801C)
#define UTXH2_REG		__REG(0x50008020)
#define URXH2_REG		__REG(0x50008024)
#define UBRDIV2_REG		__REG(0x50008028)

#define UTRSTAT_TX_EMPTY	(1 << 2)
#define UTRSTAT_RX_READY	(1 << 0)
#define UART_ERR_MASK		0xF

/*
 * PWM timer
 */
#define ELFIN_TIMER_BASE	0x51000000

#define TCFG0_REG		__REG(0x51000000)
#define TCFG1_REG		__REG(0x51000004)
#define TCON_REG		__REG(0x51000008)
#define TCNTB0_REG		__REG(0x5100000C)
#define TCMPB0_REG		__REG(0x51000010)
#define TCNTO0_REG		__REG(0x51000014)
#define TCNTB1_REG		__REG(0x51000018)
#define TCMPB1_REG		__REG(0x5100001C)
#define TCNTO1_REG		__REG(0x51000020)
#define TCNTB2_REG		__REG(0x51000024)
#define TCMPB2_REG		__REG(0x51000028)
#define TCNTO2_REG		__REG(0x5100002C)
#define TCNTB3_REG		__REG(0x51000030)
#define TCMPB3_REG		__REG(0x51000034)
#define TCNTO3_REG		__REG(0x51000038)
#define TCNTB4_REG		__REG(0x5100003C)
#define TCNTO4_REG		__REG(0x51000040)

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
#define ELFIN_RTC_BASE		0x57000000
#define RTCCON_REG		__REG(0x57000040)
#define TICNT_REG		__REG(0x57000044)
#define RTCALM_REG		__REG(0x57000050)
#define ALMSEC_REG		__REG(0x57000054)
#define ALMMIN_REG		__REG(0x57000058)
#define ALMHOUR_REG		__REG(0x5700005c)
#define ALMDATE_REG		__REG(0x57000060)
#define ALMMON_REG		__REG(0x57000064)
#define ALMYEAR_REG		__REG(0x57000068)
#define BCDSEC_REG		__REG(0x57000070)
#define BCDMIN_REG		__REG(0x57000074)
#define BCDHOUR_REG		__REG(0x57000078)
#define BCDDATE_REG		__REG(0x5700007c)
#define BCDDAY_REG		__REG(0x57000080)
#define BCDMON_REG		__REG(0x57000084)
#define BCDYEAR_REG		__REG(0x57000088)

/* USB Device */
#define ELFIN_USBD_BASE		0x52000000
#define USB_DEVICE_PHYS_ADR	ELFIN_USBD_BASE

/* include common stuff */
#ifndef __ASSEMBLY__
static inline S3C24X0_MEMCTL * S3C24X0_GetBase_MEMCTL(void)
{
	return (S3C24X0_MEMCTL *)(ELFIN_MEM_CON_BASE);
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

#define CLKCON_USBD		(1<< 15)
#define BIT19			0x00080000
#define BIT29			0x20000000
#define BIT_USBD		BIT29
#define BIT_USBD_SUB		BIT19

#define ClearPending(bit) {\
		 rSRCPND = bit;\
		 rINTPND = bit;\
		 rINTPND;\
		 }
/* Wait until rINTPND is changed for the case that the ISR is very short. */

#endif /*__S3C2440_H__*/
