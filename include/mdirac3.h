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
 * NAME	    : mdirac3.h
 *
 * Based on mDirac3 User's manual Rev 1.1
 ************************************************/

#ifndef __MDIRAC3_H__
#define __MDIRAC3_H__

#ifndef CONFIG_MDIRAC3
#define CONFIG_MDIRAC3		1
#endif

#define S3C64XX_UART_CHANNELS	3
#define S3C64XX_SPI_CHANNELS	2

#include <asm/hardware.h>

#ifndef __ASSEMBLY__
typedef enum {
	S3C64XX_UART0,
	S3C64XX_UART1,
	S3C64XX_UART2,
} S3C64XX_UARTS_NR;

#include <s3c64x0.h>
#endif

#define BIT0 		0x00000001
#define BIT1 		0x00000002
#define BIT2 		0x00000004
#define BIT3 		0x00000008
#define BIT4 		0x00000010
#define BIT5 		0x00000020
#define BIT6		0x00000040
#define BIT7		0x00000080
#define BIT8		0x00000100
#define BIT9		0x00000200
#define BIT10	0x00000400
#define BIT11	0x00000800
#define BIT12	0x00001000
#define BIT13	0x00002000
#define BIT14	0x00004000
#define BIT15	0x00008000
#define BIT16	0x00010000
#define BIT17	0x00020000
#define BIT18	0x00040000
#define BIT19	0x00080000
#define BIT20	0x00100000
#define BIT21	0x00200000
#define BIT22	0x00400000
#define BIT23	0x00800000
#define BIT24	0x01000000
#define BIT25	0x02000000
#define BIT26	0x04000000
#define BIT27	0x08000000	
#define BIT28	0x10000000
#define BIT29	0x20000000
#define BIT30	0x40000000
#define BIT31	0x80000000

#define ROM_BASE0		0x00000000      /* base address of rom bank 0 */
#define ROM_BASE1		0x04000000      /* base address of rom bank 1 */
#define DRAM_BASE0		0x30000000      /* base address of dram bank 0 */
#define DRAM_BASE1		0x40000000      /* base address of dram bank 1 */


/* MDIRAC3 device base addresses */
#define ELFIN_DMA_BASE			0x75000000
#define ELFIN_USB_OTG_BASE		0x76100000
#define ELFIN_LCD_BASE			0x77100000
#define ELFIN_USB_DEVICE_BASE	0x7f007000
#define ELFIN_USB_HOST_BASE	0x76100000	/*mDirac3 doesn't have a USB host, USB OTG included*/
#define ELFIN_I2C_BASE			0x7f003000
#define ELFIN_I2S_BASE			0x7f002000
#define ELFIN_ADC_BASE			0x7e00b000
#define ELFIN_SPI_BASE			0x7f00b000
#define ELFIN_SDI_BASE			0x7f005000

/*
 * Clock and power management
 */

/* Input Frequency */
#define FIN	12000000

/* PMS value for 400 MHz */
#define Startup_PLL_MVAL	200
#define Startup_PLL_PVAL		3
#define Startup_PLL_SVAL		1

/* Clock divisions */
#define Startup_ARM_DIV     	0
#define Startup_AXI_DIV     	3
#define Startup_APB_DIV     	1
#define Startup_MCLKx2_DIV  	1
#define Startup_MCLK_DIV    	1

#define Startup_MPLL        (((FIN>>Startup_PLL_SVAL)/Startup_PLL_PVAL)*Startup_PLL_MVAL)
#define Startup_ARMCLK   	(Startup_MPLL/(Startup_ARM_DIV+1))
#define Startup_HCLK        (Startup_ARMCLK/(Startup_AXI_DIV+1))
#define Startup_PCLK        (Startup_HCLK/(Startup_APB_DIV+1))
#define Startup_MCLK       	(Startup_MPLL/(Startup_MCLKx2_DIV+1)/(Startup_MCLK_DIV+1))

/* UART divisor value calculation */
#define Startup_UART_DIVISOR ((Startup_PCLK/(16*115200))-1)

#define PLL_MDIV			Fld(8, 16)
#define PLL_MainDiv(val)	((val) << FShft(PLL_MDIV))
#define PLL_PDIV			Fld(6, 8)
#define PLL_PreDiv(val)	((val) << FShft(PLL_PDIV))
#define PLL_SDIV			Fld(2, 0)
#define PLL_PostDiv(val)	((val) << FShft(PLL_SDIV))

#define GET_MDIV(x)		FExtr(x, PLL_MDIV)
#define GET_PDIV(x)		FExtr(x, PLL_PDIV)
#define GET_SDIV(x)		FExtr(x, PLL_SDIV)

#define CLKDIV_ARMDIV		Fld(4, 0)
#define CLKDIV_ArmDiv(val)	((val) << FShft(CLKDIV_ARMDIV))

#define CLKDIV_AXIDIV		Fld(4, 4)
#define CLKDIV_AxiDiv(val)	((val) << FShft(CLKDIV_AXIDIV))

#define CLKDIV_APBDIV		Fld(4, 8)
#define CLKDIV_ApbDiv(val)	((val) << FShft(CLKDIV_APBDIV))

#define CLKDIV_MCLKX2DIV	Fld(4, 12)
#define CLKDIV_MCLKx2Div(val)	((val) << FShft(CLKDIV_MCLKX2DIV))

#define CLKDIV_MCLKDIV		Fld(1, 16)
#define CLKDIV_MCLKDiv(val)	((val) << FShft(CLKDIV_MCLKDIV))


#define ELFIN_CLOCK_POWER_BASE	0x7e00f000

/* Clock & Power Controller for mDirac3*/
#define PLL_EN_OFFSET			0x00
#define MPLL_LOCK_OFFSET		0x04
#define UPLL_LOCK_OFFSET		0x08
#define EPLL_LOCK_OFFSET		0x0C
#define MPLL_CON_OFFSET		0x10
#define UPLL_CON_OFFSET		0x14
#define EPLL_CON_OFFSET		0x18
#define CLK_SRC_OFFSET			0x1C
#define CLK_DIV0_OFFSET			0x20
#define CLK_DIV1_OFFSET			0x24
#define CLK_OUT_OFFSET			0x28
#define HCLK_GATE_OFFSET		0x2C
#define PCLK_GATE_OFFSET		0x34
#define SCLK_GATE_OFFSET		0x3C
#define AHB_D_CON_OFFSET		0x4C
#define AHB_F_CON_OFFSET		0x50
#define AHB_I_CON_OFFSET		0x54
#define AHB_V_CON_OFFSET		0x58
#define AHB_G_CON_OFFSET		0x5C
#define AHB_M_CON_OFFSET		0x60
#define OTHERS_OFFSET			0x64
#define PWR_MODE_OFFSET		0x68
#define SW_RST_OFFSET			0x6C
#define SYS_ID_OFFSET			0x70
#define RST_CNT_VAL_OFFSET		0x800
#define OSC_CNT_VAL_OFFSET		0x804
#define PWR_CNT_VAL_OFFSET	0x808
#define FPC_CNT_VAL_OFFSET		0x80C
#define PWR_CFG_OFFSET			0x810
#define RST_STAT_OFFSET		0x814
#define WAKEUP_STAT_OFFSET		0x818
#define STOP_RETENT_OFFSET		0x81C
#define NOR_RETENT_OFFSET		0x820
#define PWR_RDY_OFFSET			0x824
#define INF_REG0_OFFSET		0x830
#define INF_REG1_OFFSET		0x834
#define INF_REG2_OFFSET		0x838
#define INF_REG3_OFFSET		0x83C



#define PLL_EN_REG			__REG(ELFIN_CLOCK_POWER_BASE+PLL_EN_OFFSET)
#define MPLL_LOCK_REG		__REG(ELFIN_CLOCK_POWER_BASE+MPLL_LOCK_OFFSET)
#define UPLL_LOCK_REG		__REG(ELFIN_CLOCK_POWER_BASE+UPLL_LOCK_OFFSET)
#define EPLL_LOCK_REG		__REG(ELFIN_CLOCK_POWER_BASE+EPLL_LOCK_OFFSET)
#define MPLL_CON_REG		__REG(ELFIN_CLOCK_POWER_BASE+MPLL_CON_OFFSET)
#define UPLL_CON_REG		__REG(ELFIN_CLOCK_POWER_BASE+UPLL_CON_OFFSET)
#define EPLL_CON_REG		__REG(ELFIN_CLOCK_POWER_BASE+EPLL_CON_OFFSET)
#define CLK_SRC_REG			__REG(ELFIN_CLOCK_POWER_BASE+CLK_SRC_OFFSET)
#define CLK_DIV0_REG		__REG(ELFIN_CLOCK_POWER_BASE+CLK_DIV0_OFFSET)
#define CLK_DIV1_REG		__REG(ELFIN_CLOCK_POWER_BASE+CLK_DIV1_OFFSET)
#define CLK_OUT_REG			__REG(ELFIN_CLOCK_POWER_BASE+CLK_OUT_OFFSET)
#define HCLK_GATE_REG		__REG(ELFIN_CLOCK_POWER_BASE+HCLK_GATE_OFFSET)
#define PCLK_GATE_REG		__REG(ELFIN_CLOCK_POWER_BASE+PCLK_GATE_OFFSET)
#define SCLK_GATE_REG		__REG(ELFIN_CLOCK_POWER_BASE+SCLK_GATE_OFFSET)
#define AHB_D_CON_REG		__REG(ELFIN_CLOCK_POWER_BASE+AHB_D_CON_OFFSET)
#define AHB_F_CON_REG		__REG(ELFIN_CLOCK_POWER_BASE+AHB_F_CON_OFFSET)
#define AHB_I_CON_REG		__REG(ELFIN_CLOCK_POWER_BASE+AHB_I_CON_OFFSET)
#define AHB_V_CON_REG		__REG(ELFIN_CLOCK_POWER_BASE+AHB_V_CON_OFFSET)
#define AHB_G_CON_REG		__REG(ELFIN_CLOCK_POWER_BASE+AHB_G_CON_OFFSET)
#define AHB_M_CON_REG		__REG(ELFIN_CLOCK_POWER_BASE+AHB_M_CON_OFFSET)
#define OTHERS_REG			__REG(ELFIN_CLOCK_POWER_BASE+OTHERS_OFFSET)
#define PWR_MODE_REG		__REG(ELFIN_CLOCK_POWER_BASE+PWR_MODE_OFFSET)
#define SW_RST_REG			__REG(ELFIN_CLOCK_POWER_BASE+SW_RST_OFFSET)
#define SYS_ID_REG			__REG(ELFIN_CLOCK_POWER_BASE+SYS_ID_OFFSET)
#define RST_CNT_VAL_REG	__REG(ELFIN_CLOCK_POWER_BASE+RST_CNT_VAL_OFFSET)
#define OSC_CNT_VAL_REG	__REG(ELFIN_CLOCK_POWER_BASE+OSC_CNT_VAL_OFFSET)
#define PWR_CNT_VAL_REG	__REG(ELFIN_CLOCK_POWER_BASE+PWR_CNT_VAL_OFFSET)
#define FPC_CNT_VAL_REG	__REG(ELFIN_CLOCK_POWER_BASE+FPC_CNT_VAL_OFFSET)
#define PWR_CFG_REG		__REG(ELFIN_CLOCK_POWER_BASE+PWR_CFG_OFFSET)
#define RST_STAT_REG		__REG(ELFIN_CLOCK_POWER_BASE+RST_STAT_OFFSET)
#define STOP_RETENT_REG	__REG(ELFIN_CLOCK_POWER_BASE+STOP_RETENT_OFFSET)
#define NOR_RETENT_REG		__REG(ELFIN_CLOCK_POWER_BASE+NOR_RETENT_OFFSET)
#define PWR_RDY_REG		__REG(ELFIN_CLOCK_POWER_BASE+PWR_RDY_OFFSET)
#define INF_REG0_REG		__REG(ELFIN_CLOCK_POWER_BASE+INF_REG0_OFFSET)
#define INF_REG1_REG		__REG(ELFIN_CLOCK_POWER_BASE+INF_REG1_OFFSET)
#define INF_REG2_REG		__REG(ELFIN_CLOCK_POWER_BASE+INF_REG2_OFFSET)
#define INF_REG3_REG		__REG(ELFIN_CLOCK_POWER_BASE+INF_REG3_OFFSET)


/* for 0x1C CLKSRC */


/*
 * Bus Matrix
 */


/*
 * GPIO
 */

#define ELFIN_GPIO_BASE		0x7f008000

#define GPA_OFFSET	0x00
#define GPB_OFFSET	0x04
#define GPC_OFFSET	0x08
#define GPD_OFFSET	0x0c
#define GPE_OFFSET	0x10
#define GPF_OFFSET	0x14
#define GPG_OFFSET	0x800
#define GPH_OFFSET	0x804
#define GPI_OFFSET	0x18

/*Except GPIO G,H*/
#define GPIO_DATA_OFFSET	0x00
#define GPIO_CON_OFFSET	0x20
#define GPIO_PU_OFFSET		0x40
#define GPIO_SLP_CON_OFFSET	0x830

/*
 * Memory controller
 */

#define ELFIN_SMC0_BASE	0x70000000
#define ELFIN_SMC1_BASE	0x70000020
#define ELFIN_SMC2_BASE	0x70000040
#define ELFIN_SMC3_BASE	0x70000060
#define ELFIN_SMC4_BASE	0x70000080
#define ELFIN_SMC5_BASE	0x700000a0
#define ELFIN_SMC6_BASE	0x700000c0
#define ELFIN_SMC7_BASE	0x700000e0


/*
 * SDRAM Controller
 */

#define ELFIN_DMC0_BASE	0x7e000000
#define ELFIN_DMC1_BASE	0x7e002000


#define INDEX_DMC_MEMC_STATUS   	(0x00)
#define INDEX_DMC_MEMC_CMD      	(0x04)
#define INDEX_DMC_DIRECT_CMD    	(0x08)
#define INDEX_DMC_MEMORY_CFG    	(0x0C)
#define INDEX_DMC_REFRESH_PRD   	(0x10)
#define INDEX_DMC_CAS_LATENCY   	(0x14)
#define INDEX_DMC_T_DQSS        		(0x18)
#define INDEX_DMC_T_MRD         		(0x1C)
#define INDEX_DMC_T_RAS         		(0x20)
#define INDEX_DMC_T_RC          		(0x24)
#define INDEX_DMC_T_RCD         		(0x28)
#define INDEX_DMC_T_RFC         		(0x2C)
#define INDEX_DMC_T_RP          		(0x30)
#define INDEX_DMC_T_RRD         		(0x34)
#define INDEX_DMC_T_WR          		(0x38)
#define INDEX_DMC_T_WTR         		(0x3C)
#define INDEX_DMC_T_XP          		(0x40)
#define INDEX_DMC_T_XSR         		(0x44)
#define INDEX_DMC_T_ESR         		(0x48)
#define INDEX_DMC_CHIP_0_CFG    	(0x200)
#define INDEX_DMC_CHIP_1_CFG    	(0x204)
#define INDEX_DMC_CHIP_2_CFG    	(0x208)
#define INDEX_DMC_CHIP_3_CFG    	(0x20C)
#define INDEX_DMC_USER_CONFIG   	(0x304)
 
/* Memory Parameters */
/* SDRAM Parameters */
#define SDR_tREFRESH      7800       /* ns */		
#define SDR_tRAS            	50          /* ns (min: 50ns)*/
#define SDR_tRC             	73          /* ns (min:72.5ns)*/
#define SDR_tRCD            	23          /* ns (min:22.5ns) */
#define SDR_tRFC           	80          /* ns (min: 80ns) */
#define SDR_tRP             	23          /* ns (min: 22.5ns) */
#define SDR_tRRD            	15          /* ns (min: 15ns)*/ 
#define SDR_tXSR            	120         /* ns (min: 120ns)*/
#define SDR_CASL		3		/* CAS Latency 3 */

#ifdef CONFIG_POP_TYPE
/* DDR Parameters */
#define DDR_tREFRESH	7500		/* ns */
#define DDR_tRAS		60			/* ns (min: 45ns)*/
#define DDR_tRC 			80			/* ns (min: 67.5ns)*/
#define DDR_tRCD		30			/* ns (min: 22.5ns)*/
#define DDR_tRFC			90			/* ns (min: 80ns)*/
#define DDR_tRP 			30			/* ns (min: 22.5ns)*/
#define DDR_tRRD		25			/* ns (min: 15ns)*/
#define DDR_tWR 			25			/* ns (min: 15ns)*/
#define DDR_tXSR			120			/* ns (min: 120ns)*/
#define DDR_CASL		3			/* CAS Latency 3 */
#else
/* DDR Parameters */
#define DDR_tREFRESH	7800		/* ns */
#define DDR_tRAS		45			/* ns (min: 45ns)*/
#define DDR_tRC 			68			/* ns (min: 67.5ns)*/
#define DDR_tRCD		23			/* ns (min: 22.5ns)*/
#define DDR_tRFC			80			/* ns (min: 80ns)*/
#define DDR_tRP 			23			/* ns (min: 22.5ns)*/
#define DDR_tRRD		15			/* ns (min: 15ns)*/
#define DDR_tWR 			15			/* ns (min: 15ns)*/
#define DDR_tXSR			120			/* ns (min: 120ns)*/
#define DDR_CASL		3			/* CAS Latency 3 */
#endif

/**************************************************************** 
 Definitions for memory configuration
 Set memory configuration
	active_chips	 = 1'b0 (1 chip)
	qos_master_chip  = 3'b000(ARID[3:0])
	memory burst	 = 3'b010(burst 4)
	stop_mem_clock	 = 1'b0(disable dynamical stop)
	auto_power_down  = 1'b0(disable auto power-down mode)
	power_down_prd	 = 6'b00_0000(0 cycle for auto power-down)
	ap_bit		 = 1'b0 (bit position of auto-precharge is 10)
	row_bits	 = 3'b010(# row address 13)
	column_bits	 = 3'b010(# column address 10 )

 Set user configuration
	2'b10=SDRAM/mSDRAM, 2'b11=DDR, 2'b01=mDDR

 Set chip select for chip [n] 
	 row bank control, bank address 0x3000_0000 ~ 0x37ff_ffff
	 CHIP_[n]_CFG=0x30F8,  30: ADDR[31:24], F8: Mask[31:24]
******************************************************************/

#define DMC0_MEM_CFG			0x00010012	//Chip1, 
#define DMC0_CHIP0_CFG			0x130f8		//0x3000_0000 ~ 0x33ff_ffff ( 64MB)
#define DMC_SDR_16_CFG			0x2 			//MWidth[3:2]=00(16bit) ,2'b10=SDRAM/mSDRAM
#define DMC_DDR_16_CFG			0x1 			//16bit, mDDR

#define DMC1_MEM_CFG			0x00010011	//Chip1, Burst4, Row/Column bit
#define DMC1_CHIP0_CFG			0x140fc		//0x4000_0000 ~ 0x43ff_ffff ( 64MB)
#define DMC_SDR_32_CFG			0x6 			//32bit, SDR
#define DMC_DDR_32_CFG			0x15 			//32bit, DDR



/*
* Memory Chip direct command
*/
#define DMC_NOP0 				0x0c0000
#define DMC_NOP1				0x1c0000
#define DMC_PA0 				0x000000	//Precharge all
#define DMC_PA1 				0x100000	
#define DMC_AR0 				0x040000	//Autorefresh 
#define DMC_AR1 				0x140000
#define DMC_SDR_MR0			0x080032	//MRS, CAS 3,  Burst Length 4
#define DMC_SDR_MR1			0x180032
#define DMC_DDR_MR0			0x080162
#define DMC_DDR_MR1			0x180162
#define DMC_mDDR_MR0			0x080032	//CAS 3, Burst Length 4
#define DMC_mDDR_MR1			0x180032
#define DMC_mSDR_EMR0			0x0a0000	//EMRS, DS:Full, PASR:Full Array 
#define DMC_mSDR_EMR1			0x1a0000
#define DMC_DDR_EMR0			0x090000
#define DMC_DDR_EMR1			0x190000
#define DMC_mDDR_EMR0			0x0a0000	// DS:Full, PASR:Full Array
#define DMC_mDDR_EMR1			0x1a0000
 

/*
* Memory Configurations for DMC
* (MCLK: DMC Clock)
*/
#define DMC_SDR_BA_EMRS 		2			
#define DMC_SDR_MEM_CASLAT	3			// Can use same value in SDRAM
#define DMC_SDR_CAS_LATENCY	(SDR_CASL<<1)		//6, 4'b0110; CASL:3,
#define DMC_SDR_t_DQSS			0				
#define DMC_SDR_t_MRD			2			
#define DMC_SDR_t_RAS			(((Startup_MCLK/1000*SDR_tRAS)-1)/1000000+1)	//7, Min 50ns
#define DMC_SDR_t_RC			(((Startup_MCLK/1000*SDR_tRC)-1)/1000000+1) //10, Min 72.5ns
#define DMC_SDR_t_RCD			(((Startup_MCLK/1000*SDR_tRCD)-1)/1000000+1) //4, Min 22.5ns
#define DMC_SDR_schedule_RCD	((DMC_SDR_t_RCD - 3) << 3)
#define DMC_SDR_t_RFC			(((Startup_MCLK/1000*SDR_tRFC)-1)/1000000+1) //11, Min 80ns 
#define DMC_SDR_schedule_RFC	((DMC_SDR_t_RFC - 3) << 5)
#define DMC_SDR_t_RP			(((Startup_MCLK/1000*SDR_tRP)-1)/1000000+1) //4, Min 22.5ns
#define DMC_SDR_schedule_RP	((DMC_SDR_t_RP - 3) << 3)
#define DMC_SDR_t_RRD			(((Startup_MCLK/1000*SDR_tRRD)-1)/1000000+1) //3, Min 15ns
#define DMC_SDR_t_WR			3			
#define DMC_SDR_t_WTR			2		
#define DMC_SDR_t_XP			2			
#define DMC_SDR_t_XSR			(((Startup_MCLK/1000*SDR_tXSR)-1)/1000000+1) // Min 120ns
#define DMC_SDR_t_ESR			DMC_SDR_t_XSR			
#define DMC_SDR_REFRESH_PRD	(((Startup_MCLK/1000*SDR_tREFRESH)-1)/1000000) // 7.8us, 1040

/*
* mDDR memory configuration
*/
#define DMC_DDR_BA_EMRS 		2			
#define DMC_DDR_MEM_CASLAT	3		
#define DMC_DDR_CAS_LATENCY	(DDR_CASL<<1)		//6   Set Cas Latency to 3
#define DMC_DDR_t_DQSS			1					// Min 0.75 ~ 1.25
#define DMC_DDR_t_MRD			2					//Min 2 tck
#define DMC_DDR_t_RAS			(((Startup_MCLK/1000*DDR_tRAS)-1)/1000000+1) //7, Min 45ns
#define DMC_DDR_t_RC			(((Startup_MCLK/1000*DDR_tRC)-1)/1000000+1) //10, Min 67.5ns
#define DMC_DDR_t_RCD			(((Startup_MCLK/1000*DDR_tRCD)-1)/1000000+1) //4,5(TRM), Min 22.5ns
#define DMC_DDR_schedule_RCD	((DMC_DDR_t_RCD -3) <<3)
#define DMC_DDR_t_RFC			(((Startup_MCLK/1000*DDR_tRFC)-1)/1000000+1) //11,18(TRM) Min 80ns
#define DMC_DDR_schedule_RFC	((DMC_DDR_t_RFC -3) <<5)
#define DMC_DDR_t_RP			(((Startup_MCLK/1000*DDR_tRP)-1)/1000000+1) //4, 5(TRM) Min 22.5ns
#define DMC_DDR_schedule_RP	((DMC_DDR_t_RP -3) << 3)
#define DMC_DDR_t_RRD			(((Startup_MCLK/1000*DDR_tRRD)-1)/1000000+1)//3, Min 15ns
#define DMC_DDR_t_WR			(((Startup_MCLK/1000*DDR_tWR)-1)/1000000+1) //Min 15ns
#define DMC_DDR_t_WTR			2					
#define DMC_DDR_t_XP			2					//1tck + tIS(1.5ns)
#define DMC_DDR_t_XSR			(((Startup_MCLK/1000*DDR_tXSR)-1)/1000000+1)//17, Min 120ns
#define DMC_DDR_t_ESR			DMC_DDR_t_XSR			
#define DMC_DDR_REFRESH_PRD	(((Startup_MCLK/1000*DDR_tREFRESH)-1)/1000000) // TRM 2656
#define DMC_DDR_USER_CONFIG	1		// 2b01 : mDDR

/*
 * SSMC 
 */
#define SMBIDCYR0       __REG(0x70000000)
#define SMBIDCYR1       __REG(0x70000020)
#define SMBIDCYR2       __REG(0x70000040)
#define SMBIDCYR3       __REG(0x70000060)
#define SMBIDCYR4       __REG(0x70000080)
#define SMBIDCYR5       __REG(0x700000A0)
#define SMBIDCYR6       __REG(0x700000C0)
#define SMBIDCYR7       __REG(0x700000E0)

#define SMBWSTRDR0      __REG(0x70000004)
#define SMBWSTRDR1      __REG(0x70000024)
#define SMBWSTRDR2      __REG(0x70000044)
#define SMBWSTRDR3      __REG(0x70000064)
#define SMBWSTRDR4      __REG(0x70000084)
#define SMBWSTRDR5      __REG(0x700000A4)
#define SMBWSTRDR6      __REG(0x700000C4)
#define SMBWSTRDR7      __REG(0x700000E4)

#define SMBWSTWRR0      __REG(0x70000008)
#define SMBWSTWRR1      __REG(0x70000028)
#define SMBWSTWRR2      __REG(0x70000048)
#define SMBWSTWRR3      __REG(0x70000068)
#define SMBWSTWRR4      __REG(0x70000088)
#define SMBWSTWRR5      __REG(0x700000A8)
#define SMBWSTWRR6      __REG(0x700000C8)
#define SMBWSTWRR7      __REG(0x700000E8)

#define SMBWSTOENR0     __REG(0x7000000C)
#define SMBWSTOENR1     __REG(0x7000002C)
#define SMBWSTOENR2     __REG(0x7000004C)
#define SMBWSTOENR3     __REG(0x7000006C)
#define SMBWSTOENR4     __REG(0x7000008C)
#define SMBWSTOENR5     __REG(0x700000AC)
#define SMBWSTOENR6     __REG(0x700000CC)
#define SMBWSTOENR7     __REG(0x700000EC)

#define SMBWSTWENR0     __REG(0x70000010)
#define SMBWSTWENR1     __REG(0x70000030)
#define SMBWSTWENR2     __REG(0x70000050)
#define SMBWSTWENR3     __REG(0x70000070)
#define SMBWSTWENR4     __REG(0x70000090)
#define SMBWSTWENR5     __REG(0x700000B0)
#define SMBWSTWENR6     __REG(0x700000D0)
#define SMBWSTWENR7     __REG(0x700000F0)

#define SMBCR0          __REG(0x70000014)
#define SMBCR1          __REG(0x70000034)
#define SMBCR2          __REG(0x70000054)
#define SMBCR3          __REG(0x70000074)
#define SMBCR4          __REG(0x70000094)
#define SMBCR5          __REG(0x700000B4)
#define SMBCR6          __REG(0x700000D4)
#define SMBCR7          __REG(0x700000F4)

#define SMBSR0          __REG(0x70000018)
#define SMBSR1          __REG(0x70000038)
#define SMBSR2          __REG(0x70000058)
#define SMBSR3          __REG(0x70000078)
#define SMBSR4          __REG(0x70000098)
#define SMBSR5          __REG(0x700000B8)
#define SMBSR6          __REG(0x700000D8)
#define SMBSR7          __REG(0x700000F8)

#define SMBWSTBDR0      __REG(0x7000001C)
#define SMBWSTBDR1      __REG(0x7000003C)
#define SMBWSTBDR2      __REG(0x7000005C)
#define SMBWSTBDR3      __REG(0x7000007C)
#define SMBWSTBDR4      __REG(0x7000009C)
#define SMBWSTBDR5      __REG(0x700000BC)
#define SMBWSTBDR6      __REG(0x700000DC)
#define SMBWSTBDR7      __REG(0x700000FC)

#define SSMCSR          __REG(0x70000200)
#define SSMCCR          __REG(0x70000204)

#define SSMCPERID0	__REG(0x70000FE0)
#define SSMCPERID1	__REG(0x70000FE4)
#define SSMCPERID2	__REG(0x70000FE8)
#define SSMCPERID3	__REG(0x70000FEC)
#define SSMCPCELLID0    __REG(0x70000FF0)
#define SSMCPCELLID1    __REG(0x70000FF4)
#define SSMCPCELLID2    __REG(0x70000FF8)
#define SSMCPCELLID3    __REG(0x70000FFC)


/*
 * Interrupt
 */
#define ELFIN_VIC0_BASE_ADDR		(0x71200000)
#define ELFIN_VIC1_BASE_ADDR		(0x71300000)
#define oINTMOD				(0x0C)  // VIC INT SELECT (IRQ or FIQ)
#define oINTMSK				(0x14)  //VIC INT EN CLEAR (Mask by writing 1)
#define oINTUNMSK			(0x10)  //VIC INT EN (Unmask by writing 1)
#define oINTSUBMSK			(0x1C)  //VIC SOFT INT CLEAR
#define oVECTADDR			(0xF00)  // VIC ADDRESS

/*
 * Watchdog timer
 */

#define ELFIN_WATCHDOG_BASE	0x7E005000

#define WTCON_REG		__REG(0x7E005004)
#define WTDAT_REG		__REG(0x7E005008)
#define WTCNT_REG		__REG(0x7E00500C)


/*
 * UART
 */
#define ELFIN_UART_BASE	0x7E004000

#define ELFIN_UART0_OFFSET	0x0000
#define ELFIN_UART1_OFFSET	0x0400
#define ELFIN_UART2_OFFSET	0x0800


#define ULCON_OFFSET	0x00
#define UCON_OFFSET	0x04
#define UFCON_OFFSET	0x08
#define UMCON_OFFSET	0x0C
#define UTRSTAT_OFFSET	0x10
#define UERSTAT_OFFSET	0x14
#define UFSTAT_OFFSET	0x18
#define UMSTAT_OFFSET	0x1C
#define UTXH_OFFSET		0x20
#define URXH_OFFSET		0x24
#define UBRDIV_OFFSET	0x28
#define UDIVSLOT_OFFSET	0x2C
#define UINTP_OFFSET	0x30
#define UINTSP_OFFSET	0x34
#define UINTM_OFFSET	0x38

#define ULCON0_REG		__REG(0x7E004000)
#define UCON0_REG		__REG(0x7E004004)
#define UFCON0_REG		__REG(0x7E004008)
#define UMCON0_REG		__REG(0x7E00400C)
#define UTRSTAT0_REG	__REG(0x7E004010)
#define UERSTAT0_REG	__REG(0x7E004014)
#define UFSTAT0_REG		__REG(0x7E004018)
#define UMSTAT0_REG	__REG(0x7E00401c)
#define UTXH0_REG		__REG(0x7E004020)
#define URXH0_REG		__REG(0x7E004024)
#define UBRDIV0_REG		__REG(0x7E004028)
#define UDIVSLOT0_REG	__REG(0x7E00402c)
#define UINTP0_REG		__REG(0x7E004030)
#define UINTSP0_REG		__REG(0x7E004034)
#define UINTM0_REG		__REG(0x7E004038)

#define ULCON1_REG		__REG(0x7E004400)
#define UCON1_REG		__REG(0x7E004404)
#define UFCON1_REG		__REG(0x7E004408)
#define UMCON1_REG		__REG(0x7E00440C)
#define UTRSTAT1_REG	__REG(0x7E004410)
#define UERSTAT1_REG	__REG(0x7E004414)
#define UFSTAT1_REG		__REG(0x7E004418)
#define UMSTAT1_REG	__REG(0x7E00441c)
#define UTXH1_REG		__REG(0x7E004420)
#define URXH1_REG		__REG(0x7E004424)
#define UBRDIV1_REG		__REG(0x7E004428)
#define UDIVSLOT1_REG	__REG(0x7E00442c)
#define UINTP1_REG		__REG(0x7E004430)
#define UINTSP1_REG		__REG(0x7E004434)
#define UINTM1_REG		__REG(0x7E004438)


#define UTRSTAT_TX_EMPTY	BIT2
#define UTRSTAT_RX_READY	BIT0
#define UART_ERR_MASK		0xF

/*
 * PWM timer
 */
#define ELFIN_TIMER_BASE	0x7e007000

#define TCNTCLRINT0_OFFSET	0x44
#define TCNTCLRINT1_OFFSET	0x48
#define TCNTCLRINT2_OFFSET	0x4c
#define TCNTCLRINT3_OFFSET	0x50
#define TCNTCLRINT4_OFFSET	0x54

#define TCFG0_REG		__REG(0x7e007000)
#define TCFG1_REG		__REG(0x7e007004)
#define TCON_REG		__REG(0x7e007008)
#define TCNTB0_REG		__REG(0x7e00700c)
#define TCMPB0_REG		__REG(0x7e007010)
#define TCNTO0_REG		__REG(0x7e007014)
#define TCNTB1_REG		__REG(0x7e007018)
#define TCMPB1_REG		__REG(0x7e00701c)
#define TCNTO1_REG		__REG(0x7e007020)
#define TCNTB2_REG		__REG(0x7e007024)
#define TCMPB2_REG		__REG(0x7e007028)
#define TCNTO2_REG		__REG(0x7e00702c)
#define TCNTB3_REG		__REG(0x7e007030)
#define TCMPB3_REG		__REG(0x7e007034)
#define TCNTO3_REG		__REG(0x7e007038)
#define TCNTB4_REG		__REG(0x7e00703c)
#define TCNTO4_REG		__REG(0x7e007040)

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
#define ELFIN_RTC_BASE	0x7e006000

#define RTCCON_REG		__REG(0x7e006040)
#define TICNT_REG		__REG(0x7e006044)
#define RTCALM_REG		__REG(0x7e006050)
#define ALMSEC_REG		__REG(0x7e006054)
#define ALMMIN_REG		__REG(0x7e006058)
#define ALMHOUR_REG	__REG(0x7e00605c)
#define ALMDATE_REG	__REG(0x7e006060)
#define ALMMON_REG		__REG(0x7e006064)
#define ALMYEAR_REG	__REG(0x7e006068)

#define RTCRST_REG		__REG(0x7e00606c)

#define BCDSEC_REG		__REG(0x7e006070)
#define BCDMIN_REG		__REG(0x7e006074)
#define BCDHOUR_REG	__REG(0x7e006078)
#define BCDDATE_REG	__REG(0x7e00607c)
#define BCDDAY_REG		__REG(0x7e006080)
#define BCDMON_REG		__REG(0x7e006084)
#define BCDYEAR_REG		__REG(0x7e006088)

/* USB Device */
#define USB_DEVICE_PHYS_ADR	0x7f007000

/* include common stuff */
#ifndef __ASSEMBLY__
static inline S3C64XX_MEMCTL * S3C64XX_GetBase_MEMCTL(void)
{
	return (S3C64XX_MEMCTL *)(ELFIN_DMC0_BASE);
}
static inline S3C64XX_USB_HOST * S3C64XX_GetBase_USB_HOST(void)
{
	return (S3C64XX_USB_HOST *)ELFIN_USB_HOST_BASE;
}
static inline S3C64XX_INTERRUPT * S3C64XX_GetBase_INTERRUPT(void)
{
	return (S3C64XX_INTERRUPT *)ELFIN_VIC0_BASE_ADDR;
}
static inline S3C64XX_DMAS * S3C64XX_GetBase_DMAS(void)
{
	return (S3C64XX_DMAS *)ELFIN_DMA_BASE;
}
static inline S3C64XX_CLOCK_POWER * S3C64XX_GetBase_CLOCK_POWER(void)
{
	return (S3C64XX_CLOCK_POWER *)ELFIN_CLOCK_POWER_BASE;
}
static inline S3C64XX_LCD * S3C64XX_GetBase_LCD(void)
{
	return (S3C64XX_LCD *)ELFIN_LCD_BASE;
}
/*
static inline S3C2410_NAND * S3C2410_GetBase_NAND(void)
{
	return (S3C2410_NAND *)ELFIN_NAND_BASE;
}
*/
static inline S3C64XX_UART * S3C64XX_GetBase_UART(S3C64XX_UARTS_NR nr)
{
	return (S3C64XX_UART *)(ELFIN_UART_BASE + (nr * 0x4000));
}
static inline S3C64XX_TIMERS * S3C64XX_GetBase_TIMERS(void)
{
	return (S3C64XX_TIMERS *)ELFIN_TIMER_BASE;
}
static inline S3C64XX_USB_DEVICE * S3C64XX_GetBase_USB_DEVICE(void)
{
	return (S3C64XX_USB_DEVICE *)ELFIN_USB_DEVICE_BASE;
}
static inline S3C64XX_WATCHDOG * S3C64XX_GetBase_WATCHDOG(void)
{
	return (S3C64XX_WATCHDOG *)ELFIN_WATCHDOG_BASE;
}
static inline S3C64XX_I2C * S3C64XX_GetBase_I2C(void)
{
	return (S3C64XX_I2C *)ELFIN_I2C_BASE;
}
static inline S3C64XX_I2S * S3C64XX_GetBase_I2S(void)
{
	return (S3C64XX_I2S *)ELFIN_I2S_BASE;
}
static inline S3C64XX_GPIO * S3C64XX_GetBase_GPIO(void)
{
	return (S3C64XX_GPIO *)ELFIN_GPIO_BASE;
}
static inline S3C64XX_RTC * S3C64XX_GetBase_RTC(void)
{
	return (S3C64XX_RTC *)ELFIN_RTC_BASE;
}
static inline S3C2410_ADC * S3C2410_GetBase_ADC(void)
{
	return (S3C2410_ADC *)ELFIN_ADC_BASE;
}
static inline S3C64XX_SPI * S3C64XX_GetBase_SPI(void)
{
	return (S3C64XX_SPI *)ELFIN_SPI_BASE;
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

#endif /*__MDIRAC3_H__*/
