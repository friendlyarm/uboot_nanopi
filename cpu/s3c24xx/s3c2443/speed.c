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

/* This code should work for both the S3C2400 and the S3C2410
 * as they seem to have the same PLL and clock machinery inside.
 * The different address mapping is handled by the s3c24xx.h files below.
 */

#include <common.h>
#include <s3c2443.h>

#define MPLL 0
#define UPLL 1

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
#if 0
	else if (pllreg == UPLL)
		r = UPLLCON_REG;
#endif
	else
		hang();

	m = ((r>>16) & 0xff) + 8;
	p = ((r>>8) & 0x3);
	s = r & 0x3;

	/* XXX: To Do calculate UPLL */

	return (2 * m * (CONFIG_SYS_CLK_FREQ / (p << s)));
}

/* return FCLK frequency */
ulong get_FCLK(void)
{
	return(get_PLLCLK(MPLL));
}

/* return HCLK frequency */
ulong get_HCLK(void)
{
	uint hclk_div = (CLKDIV0CON_REG & 0x3) + 1;
	uint pre_div = ((CLKDIV0CON_REG>>4) & 0x3) + 1;

	return get_FCLK()/(hclk_div * pre_div);
}

/* return PCLK frequency */
ulong get_PCLK(void)
{
	return ((CLKDIV0CON_REG & 0x4) ? get_HCLK()/2 : get_HCLK());
}

/* return UCLK frequency */
ulong get_UCLK(void)
{
	return (get_PLLCLK(UPLL));
}

/* return ARMCORE frequency */
ulong get_ARMCLK(void)
{
	uint div_val = CLKDIV0CON_REG;
	uint fclk = get_FCLK();
	uint arm_clk = 0;

	switch((div_val >> 9) & 0xf) {
	case 0:
		arm_clk = fclk;
		break;

	case 8:
		arm_clk = fclk>>1;
		break;

	case 2:
		arm_clk = fclk/3;
		break;

	case 9:
		arm_clk = fclk>>2;
		break;

	case 10:
		arm_clk = fclk/6;
		break;

	case 11:
		arm_clk = fclk>>3;
		break;

	case 13:
		arm_clk = fclk/12;
		break;

	case 15:
		arm_clk = fclk>>4;
		break;

	}

	return arm_clk;
}

int print_cpuinfo(void)
{
	printf("\nCPU:   S3C2443@%dMHz\n", get_ARMCLK()/1000000);
	printf("       Fclk = %dMHz, Hclk = %dMHz, Pclk = %dMHz\n",
			get_FCLK()/1000000, get_HCLK()/1000000, get_PCLK()/1000000);

	return 0;
}
