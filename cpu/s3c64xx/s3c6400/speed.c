/*
 * (C) Copyright 2001-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, d.mueller@elsoft.ch
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

/* This code should work for only S3C6400.
 * You should make sure that S3C6400 has only one clock mode of ASYNC.
 * But, S3C6410 has 2 clock modes of SYNC and ASYNC.
 * When you are using S3C6400 and 6410, you should bear in mind the difference
 * of 2 chips.
 * by scsuh.
 */

#include <common.h>
#include <s3c6400.h>

#define APLL 0
#define MPLL 1
#define EPLL 2

/* ------------------------------------------------------------------------- */
/* NOTE: This describes the proper use of this file.
 *
 * CONFIG_SYS_CLK_FREQ should be defined as the input frequency of the PLL.
 *
 * get_FCLK(), get_HCLK(), get_PCLK() and get_UCLK() return the clock of
 * the specified bus in HZ.
 */
/* ------------------------------------------------------------------------- */

static ulong get_PLLCLK(int pllreg)
{
	ulong r, m, p, s;

	if (pllreg == APLL)
		r = APLL_CON_REG;
	else if (pllreg == MPLL)
		r = MPLL_CON_REG;
	else if (pllreg == EPLL)
		r = EPLL_CON0_REG;
	else
		hang();

	m = (r>>16) & 0x3ff;
	p = (r>>8) & 0x3f;
	s = r & 0x7;

	return (m * (CONFIG_SYS_CLK_FREQ / (p * (1 << s))));
}

/* return ARMCORE frequency */
ulong get_ARMCLK(void)
{
	ulong div;

	div = CLK_DIV0_REG;

	return (get_PLLCLK(APLL) / ((div & 0x7) + 1));
}

/* return FCLK frequency */
ulong get_FCLK(void)
{
	return (get_PLLCLK(APLL));
}

/* return HCLK frequency */
ulong get_HCLK(void)
{
	ulong fclk;

	uint hclkx2_div = ((CLK_DIV0_REG>>9) & 0x7) + 1;
	uint hclk_div = ((CLK_DIV0_REG>>8) & 0x1) + 1;

	/* S3C6400 does not allow SYNC mode. So, code below is
	 * meanless at this time. by scsuh.
	 * only support ASYNC mode.
	 */
#if 0
	if(OTHERS_REG & 0x80)
		fclk = get_FCLK();		// SYNC Mode
	else
		fclk = get_PLLCLK(MPLL);	// ASYNC Mode
#else
	fclk = get_PLLCLK(MPLL);	/* ASYNC Mode */
#endif
	return fclk/(hclk_div * hclkx2_div);
}

/* return PCLK frequency */
ulong get_PCLK(void)
{
	ulong fclk;
	uint hclkx2_div = ((CLK_DIV0_REG>>9) & 0x7) + 1;
	uint pre_div = ((CLK_DIV0_REG>>12) & 0xf) + 1;

	/* S3C6400 does not allow SYNC mode. So, code below is
	 * meanless at this time. by scsuh.
	 * only support ASYNC mode.
	 */
#if 0
	if(OTHERS_REG & 0x80)
		fclk = get_FCLK();		// SYNC Mode
	else
		fclk = get_PLLCLK(MPLL);	// ASYNC Mode
#else
	fclk = get_PLLCLK(MPLL);	/* ASYNC Mode */
#endif
	return fclk/(hclkx2_div * pre_div);
}

/* return UCLK frequency */
ulong get_UCLK(void)
{
	return (get_PLLCLK(EPLL));
}

int print_cpuinfo(void)
{
	printf("\nCPU:     S3C6400@%dMHz\n", get_ARMCLK()/1000000);
	printf("         Fclk = %dMHz, Hclk = %dMHz, Pclk = %dMHz ",
			get_FCLK()/1000000, get_HCLK()/1000000, get_PCLK()/1000000);
	/*
	 * Display Serial SRC
	 */
#if defined(CONFIG_CLKSRC_CLKUART)
	puts(", Serial = CLKUART ");
#else
	puts(", Serial = PCLK ");
#endif

	/* S3C6400 does not allow SYNC mode. So, code below is
	 * meanless at this time. by scsuh.
	 * only support ASYNC mode.
	 */
#if 0
	if(OTHERS_REG & 0x80)
		printf("(SYNC Mode) \n");
	else
		printf("(ASYNC Mode) \n");
#else
	/* OTHERS[7] is used to check clock mode.
	 * when this bit is 1b clock mode is wrong.
	 * by scsuh.
	 */
	if (OTHERS_REG & 0x80)
		printf("\n####### this clock mode is not allowed. #######\n");
	else
		printf("(ASYNC Mode) \n");
#endif
	return 0;
}
