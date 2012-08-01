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
 * NAME	    : s3c24a0.h
 *
 * Based on S3C24A0X User's manual Rev 1.1
 ************************************************/

#ifndef __S3C24A0_H__
#define __S3C24A0_H__

#ifndef CONFIG_S3C24A0
#define CONFIG_S3C24A0		1
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

#define ROM_BASE0			0x00000000      /* base address of rom bank 0 */
#define ROM_BASE1			0x04000000      /* base address of rom bank 1 */
#define DRAM_BASE0			0x10000000      /* base address of dram bank 0 */
#define DRAM_BASE1			0x20000000      /* base address of dram bank 1 */

/* S3C24A0 device base addresses */
#define S3C24A0_DMA_BASE		0x40400000
#define ELFIN_USB_HOST_BASE		0x41000000
#define ELFIN_LCD_BASE			0x4a000000
#define S3C24A0_USB_DEVICE_BASE		0x44a00000
#define S3C24A0_I2C_BASE		0x54000000
#define S3C24A0_I2S_BASE		0x55000000
#define S3C24A0_ADC_BASE		0x58000000
#define S3C24A0_SPI_BASE		0x59000000
#define S3C2410_SDI_BASE		0x5A000000


/*
 * Clock and power management
 */

#define ELFIN_CLOCK_POWER_BASE	0x40000000

#define LOCKTIME_OFFSET		0x00
#define XTALWSET_OFFSET		0x04
#define MPLLCON_OFFSET		0x10
#define UPLLCON_OFFSET		0x14
#define CLKCON_OFFSET		0x20
#define CLKSRC_OFFSET		0x24
#define CLKDIVN_OFFSET		0x28
#define PWRMAN_OFFSET		0x30
#define SRESET_OFFSET		0x38

#define LOCKTIME_REG		__REG(0x40000000)
#define XTALWSET_REG		__REG(0x40000004)
#define MPLLCON_REG		__REG(0x40000010)
#define UPLLCON_REG		__REG(0x40000014)
#define CLKCON_REG		__REG(0x40000020)
#define CLKSRC_REG		__REG(0x40000024)
#define CLKDIV_REG		__REG(0x40000028)
#define PWRMAN_REG		__REG(0x40000030)
#define SOFTRESET_REG		__REG(0x40000038)

#define CLKCON_USBD		(1<< 15)

#define fPLL_MDIV		Fld(8,12)
#define fPLL_PDIV		Fld(6,4)
#define fPLL_SDIV		Fld(2,0)

#define PLL_MDIV		Fld(8, 12)
#define PLL_MainDiv(val)	((val) << FShft(PLL_MDIV))
#define PLL_PDIV		Fld(6, 4)
#define PLL_PreDiv(val)		((val) << FShft(PLL_PDIV))
#define PLL_SDIV		Fld(2, 0)
#define PLL_PostDiv(val)	((val) << FShft(PLL_SDIV))
#define GET_MDIV(x)		FExtr(x, PLL_MDIV)
#define GET_PDIV(x)		FExtr(x, PLL_PDIV)
#define GET_SDIV(x)		FExtr(x, PLL_SDIV)

#define CLKDIV_HCLKDIV		Fld(1, 1)
#define CLKDIV_HclkDiv(val)	((val) << FShft(CLKDIV_HCLKDIV))
#define CLKDIV_PCLKDIV		Fld(1, 0)
#define CLKDIV_PclkDiv(val)	((val) << FShft(CLKDIV_PCLKDIV))


/*
 * Interrupt
 */
#define ELFIN_INTERRUPT_BASE	0x40200000

#define SRCPND_OFFSET		0x00
#define INTMOD_OFFSET		0x04
#define INTMSK_OFFSET		0x08
#define PRIORITY_OFFSET	0x0C
#define INTPND_OFFSET		0x10
#define INTOFFSET_OFFSET	0x14
#define SUBSRCPND_OFFSET	0x18
#define INTSUBMSK_OFFSET	0x1C
#define INTVECTMODE_OFFSET	0x20
#define INTVAR_OFFSET		0x2C

#define SRCPND_REG		__REG(0x40200000)
#define INTMOD_REG		__REG(0x40200004)
#define INTMSK_REG		__REG(0x40200008)
#define INTPRIORITY_REG		__REG(0x4020000C)
#define INTPND_REG		__REG(0x40200010)
#define INTOFFSET_REG		__REG(0x40200014)
#define SUBSRCPND_REG		__REG(0x40200018)
#define INTSUBMSK_REG		__REG(0x4020001C)
#define INTVECTMODE_REG		__REG(0x40200020)
#define INTVAR_REG		__REG(0x4020002C)

#define EXTINTCR0_REG		__REG(0x44800018)
#define EXTINTCR1_REG		__REG(0x4480001c)
#define EXTINTCR2_REG		__REG(0x44800020)
#define EINTMASK_REG		__REG(0x44800034)
#define EINTPEND_REG		__REG(0x44800038)


/*
 * Watchdog
 */
#define ELFIN_WATCHDOG_BASE	0x44100000
#define WTCON_OFFSET		0x00
#define WTDAT_OFFSET		0x04
#define WTCNT_OFFSET		0x08


/*
 * GPIO
 */

#define S3C24A0_GPIO_BASE	0x44800000
#define GPCON_U_OFFSET		0x00
#define GPCON_M_OFFSET		0x04
#define GPCON_L_OFFSET		0x08
#define GPDAT_OFFSET		0x0c
#define EXTINTC0_OFFSET		0x18
#define EXTINTC1_OFFSET		0x1c
#define EXTINTC2_OFFSET		0x20

#define ALIVECON_OFFSET		0x44

#define GPRAM0_OFFSET		0x80

#define GPCON_U_REG		__REG((S3C24A0_GPIO_BASE+GPCON_U_OFFSET))
#define GPCON_M_REG		__REG((S3C24A0_GPIO_BASE+GPCON_M_OFFSET))
#define GPCON_L_REG		__REG((S3C24A0_GPIO_BASE+GPCON_L_OFFSET))
#define GPDAT_REG		__REG((S3C24A0_GPIO_BASE+GPDAT_OFFSET))
#define ALIVECON_REG		__REG((S3C24A0_GPIO_BASE+ALIVECON_OFFSET))
#define GPRAM0_REG		__REG((S3C24A0_GPIO_BASE+GPRAM0_OFFSET))

/*
 * Memory controller
 */

#define ELFIN_SROM_BASE		0x40c20000

#define SROM_BW_REG		__REG(0x40c20000)
#define SROM_BC0_REG		__REG(0x40c20004)
#define SROM_BC1_REG		__REG(0x40c20008)
#define SROM_BC2_REG		__REG(0x40c2000C)

#if 1
#define SROM_TACS		Fld(2, 14)
#define SROM_Tacs(clk)		((clk) << FShft(SROM_TACS))
#define SROM_Tacs_0clks		(0x0)
#define SROM_Tacs_2clks		(0x1)
#define SROM_Tacs_4clks		(0x2)
#define SROM_Tacs_8clks		(0x3)

#define SROM_TCOS		Fld(2, 12)
#define SROM_Tcos(clk)		((clk) << FShft(SROM_TCOS))
#define SROM_Tcos_0clks		(0x0)
#define SROM_Tcos_2clks		(0x1)
#define SROM_Tcos_4clks		(0x2)
#define SROM_Tcos_8clks		(0x3)

#define SROM_TACC			Fld(3, 8)
#define SROM_Tacc(clk)		((clk) << FShft(SROM_TACC))
#define SROM_Tacc_2clks		(0x0)
#define SROM_Tacc_3clks		(0x1)
#define SROM_Tacc_4clks		(0x2)
#define SROM_Tacc_10clks	(0x3)
#define SROM_Tacc_12clks	(0x4)
#define SROM_Tacc_14clks	(0x5)
#define SROM_Tacc_16clks	(0x6)
#define SROM_Tacc_20clks	(0x7)

#define SROM_TCOH			Fld(2, 6)
#define SROM_Tcoh(clk)		((clk) << FShft(SROM_TCOH))
#define SROM_Tcoh_0clks		(0x0)
#define SROM_Tcoh_2clks		(0x1)
#define SROM_Tcoh_4clks		(0x2)
#define SROM_Tcoh_8clks		(0x3)

#define SROM_TCAH			Fld(2, 4)
#define SROM_Tcah(clk)		((clk) << FShft(SROM_TCAH))
#define SROM_Tcah_0clks		(0x0)
#define SROM_Tcah_2clks		(0x1)
#define SROM_Tcah_4clks		(0x2)
#define SROM_Tcah_8clks		(0x3)

#define SROM_TACP			Fld(2, 2)
#define SROM_Tacp(clk)		((clk) << FShft(SROM_TACP))
#define SROM_Tacp_2clks		(0x0)
#define SROM_Tacp_4clks		(0x1)
#define SROM_Tacp_8clks		(0x2)
#define SROM_Tacp_10clks	(0x3)

#define SROM_PMC			Fld(2, 0)
#define SROM_Pmc(data)		((data) << FShft(SROM_PMC))
#define SROM_Pmc_1data		(0x0)
#define SROM_Pmc_4data		(0x1)
#define SROM_Pmc_8data		(0x2)
#define SROM_Pmc_16data		(0x3)
#endif

/*
 * SDRAM Controller
 */
#define ELFIN_MEMCTL_BASE	0x40c40000
#define SDRAM_BANKCFG_OFFSET	0x00
#define SDRAM_BANKCON_OFFSET	0x04
#define SDRAM_REFRESH_OFFSET	0x08
#define SDRAM_TIMEOUT_OFFSET	0x0c

#define SDRAM_DS_SHT		29
#define SDRAM_TCSR_SHT		27
#define SDRAM_PASR_SHT		24
#define SDRAM_PWRDN_SHT		20
#define SDRAM_Tras_SHT		16
#define SDRAM_Trc_SHT		12
#define SDRAM_Trcd_SHT		10
#define SDRAM_Trp_SHT		8
#define SDRAM_DENSITY1_SHT	6
#define SDRAM_DENSITY0_SHT	4
#define SDRAM_CL_SHT		2
#define SDRAM_P_SHT		1
#define SDRAM_DW_SHT		0


/*
 * Nand flash controller
 */

#define ELFIN_NAND_BASE	0x40c00000

#define NFCONF_OFFSET		0x00
#define NFCONT_OFFSET		0x04
#define NFCMD_OFFSET		0x08
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


/*
 * Watchdog timer
 */

#define WTCON_REG		__REG(0x44100000)
#define WTDAT_REG		__REG(0x44100004)
#define WTCNT_REG		__REG(0x44100008)


/*
 * UART
 */
#define S3C24A0_UART_BASE	0x44400000

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
#define S3C24A0_TIMER_BASE	0x44000000

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
#define S3C24A0_RTC_BASE	0x44200000
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
static inline S3C24X0_MEMCTL * const S3C24X0_GetBase_MEMCTL(void)
{
	return (S3C24X0_MEMCTL * const)ELFIN_MEMCTL_BASE;
}
static inline S3C24X0_USB_HOST * const S3C24X0_GetBase_USB_HOST(void)
{
	return (S3C24X0_USB_HOST * const)ELFIN_USB_HOST_BASE;
}
static inline S3C24X0_INTERRUPT * const S3C24X0_GetBase_INTERRUPT(void)
{
	return (S3C24X0_INTERRUPT * const)ELFIN_INTERRUPT_BASE;
}
static inline S3C24X0_DMAS * const S3C24X0_GetBase_DMAS(void)
{
	return (S3C24X0_DMAS * const)S3C24A0_DMA_BASE;
}
static inline S3C24X0_CLOCK_POWER * const S3C24X0_GetBase_CLOCK_POWER(void)
{
	return (S3C24X0_CLOCK_POWER * const)ELFIN_CLOCK_POWER_BASE;
}
static inline S3C24X0_LCD * const S3C24X0_GetBase_LCD(void)
{
	return (S3C24X0_LCD * const)ELFIN_LCD_BASE;
}
static inline S3C2410_NAND * const S3C2410_GetBase_NAND(void)
{
	return (S3C2410_NAND * const)ELFIN_NAND_BASE;
}
static inline S3C24X0_UART * const S3C24X0_GetBase_UART(S3C24X0_UARTS_NR nr)
{
	return (S3C24X0_UART * const)(S3C24A0_UART_BASE + (nr * 0x4000));
}
static inline S3C24X0_TIMERS * const S3C24X0_GetBase_TIMERS(void)
{
	return (S3C24X0_TIMERS * const)S3C24A0_TIMER_BASE;
}
static inline S3C24X0_USB_DEVICE * const S3C24X0_GetBase_USB_DEVICE(void)
{
	return (S3C24X0_USB_DEVICE * const)S3C24A0_USB_DEVICE_BASE;
}
static inline S3C24X0_WATCHDOG * const S3C24X0_GetBase_WATCHDOG(void)
{
	return (S3C24X0_WATCHDOG * const)ELFIN_WATCHDOG_BASE;
}
static inline S3C24X0_I2C * const S3C24X0_GetBase_I2C(void)
{
	return (S3C24X0_I2C * const)S3C24A0_I2C_BASE;
}
static inline S3C24X0_I2S * const S3C24X0_GetBase_I2S(void)
{
	return (S3C24X0_I2S * const)S3C24A0_I2S_BASE;
}
static inline S3C24X0_GPIO * const S3C24X0_GetBase_GPIO(void)
{
	return (S3C24X0_GPIO * const)S3C24A0_GPIO_BASE;
}
static inline S3C24X0_RTC * const S3C24X0_GetBase_RTC(void)
{
	return (S3C24X0_RTC * const)S3C24A0_RTC_BASE;
}
static inline S3C2410_ADC * const S3C2410_GetBase_ADC(void)
{
	return (S3C2410_ADC * const)S3C24A0_ADC_BASE;
}
static inline S3C24X0_SPI * const S3C24X0_GetBase_SPI(void)
{
	return (S3C24X0_SPI * const)S3C24A0_SPI_BASE;
}
static inline S3C2410_SDI * const S3C2410_GetBase_SDI(void)
{
	return (S3C2410_SDI * const)S3C2410_SDI_BASE;
}
#else /* #ifndef __ASSEMBLY__ */

/* LCD controller */
#define LCDBGCON_OFFSET		0x5c
#endif /* #ifndef __ASSEMBLY__ */

/* PENDING BIT */
#define BIT_EINT0		(0x1)
#define BIT_EINT1		(0x1<<1)
#define BIT_EINT2		(0x1<<2)
#define BIT_EINT3		(0x1<<3)
#define BIT_EINT4_7		(0x1<<4)
#define BIT_EINT8_23	(0x1<<5)
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
#define BIT_USBD		(0x1<<25)
#define BIT_USBH		(0x1<<26)
#define BIT_IIC			(0x1<<27)
#define BIT_UART0		(0x1<<28)
#define BIT_SPI1		(0x1<<29)
#define BIT_RTC			(0x1<<30)
#define BIT_ADC			(0x1<<31)
#define BIT_ALLMSK		(0xFFFFFFFF)

#define ClearPending(bit) {\
		 rSRCPND = bit;\
		 rINTPND = bit;\
		 rINTPND;\
		 }
/* Wait until rINTPND is changed for the case that the ISR is very short. */

#endif /*__S3C24A0_H__*/
