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

#include <common.h>
#include <s3c2440.h>

#define MPLL 0
#define UPLL 1

#ifdef CONFIG_S3C2440
static char cpu_name[] = "S3C2440A";
#elif defined (CONFIG_S3C2442)
static char cpu_name[] = "S3C2442";
#else
#error no cpu type is defined.
#endif

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

	if (pllreg == MPLL)
		r = MPLLCON_REG;
	else if (pllreg == UPLL)
		r = UPLLCON_REG;
	else
		hang();

	m = ((r>>12) & 0xff) + 8;
	p = ((r>>4) & 0x3f) + 2;
	s = r & 0x3;

	/* XXX: To Do calculate UPLL */

	return (2 * m * (CONFIG_SYS_CLK_FREQ / (p << s)));
}

/* return FCLK frequency */
ulong get_FCLK(void)
{
#ifndef  S3C2440_PLL_OFF
	return(get_PLLCLK(MPLL));
#else
	return CONFIG_SYS_CLK_FREQ;
#endif
}

/* return HCLK frequency */
ulong get_HCLK(void)
{
	switch(CLKDIVN_REG & 0x6) {
	case 0x0:
		return (get_FCLK());
	case 0x2:
		return (get_FCLK()/2);
	case 0x4:
		return((CAMDIVN_REG & 0x200) ? get_FCLK()/8 : get_FCLK()/4);
	case 0x6:
		return((CAMDIVN_REG & 0x100) ? get_FCLK()/6 : get_FCLK()/3);
	}

	return 0;
}

/* return PCLK frequency */
ulong get_PCLK(void)
{
	return ((CLKDIVN_REG & 0x1) ? get_HCLK()/2 : get_HCLK());
}

/* return UCLK frequency */
ulong get_UCLK(void)
{
	return (get_PLLCLK(UPLL));
}

/* return ARMCORE frequency */
ulong get_ARMCLK(void)
{
	return get_FCLK();
}

int print_cpuinfo(void)
{
	printf("\nCPU:   %s@%dMHz\n", cpu_name, get_ARMCLK()/1000000);
	printf("       Fclk = %dMHz, Hclk = %dMHz, Pclk = %dMHz\n",
			get_FCLK()/1000000, get_HCLK()/1000000, get_PCLK()/1000000);

	return 0;
}
