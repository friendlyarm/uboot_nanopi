#ifndef _S3C2442_SMDK_H
#define _S3C2442_SMDK_H

#include <config.h>
#include <version.h>

#include <s3c2442.h>

#if (CONFIG_SYS_CLK_FREQ == 12000000)
/* this mode is used for KingFISH */
#define MDIV_406               195
#define PDIV_406               4
#define SDIV_406               1

#define MDIV_405               127
#define PDIV_405               2
#define SDIV_405               1

#define MDIV_96                56
#define PDIV_96                2
#define SDIV_96                1

#define MDIV_48                56
#define PDIV_48                2
#define SDIV_48                2

#define CLKDIVN_136            0xf  /* UCLK = UPLL/2 */
#define CLKDIVN_148            0x5  /* UCLK = UPLL/2 */

#define UPLL_VAL		((MDIV_48 << 12) | (PDIV_48 << 4) | (SDIV_48))
#define MPLL_VAL       		((MDIV_406 << 12) | (PDIV_406 << 4) | (SDIV_406))
#define CLKDIVN_VAL		CLKDIVN_148
#elif (CONFIG_SYS_CLK_FREQ == 16934400)
#define MPLL_VAL	0x0003f040	/* 0x61012: 296, 98, 49 */ /* 0x0003f040: 400, 133, 66 */
#define UPLL_VAL	0x0003c041
#define CLKDIVN_VAL	0xf
#else
#error unknown system clock in smdk2442_val.h
#endif

#define vBWSCON		0x2211d120
#define vBANKCON0	0x00000700
#define vBANKCON1	0x00000700
#define vBANKCON2	0x00000700
#define vBANKCON3	0x00001f4c
#define vBANKCON4	0x00000700
#define vBANKCON5	0x00000700
#define vBANKCON6	0x00018005
#define vBANKCON7	0x00018005

#if defined(CONFIG_SMDK2442)
#define vBANKSIZE	0x00000031
#define vREFRESH	0x008e04f5
#elif defined(CONFIG_SP3000)
#define vBANKSIZE	0x00000022
#define vREFRESH	0x008e07a3
#else
#error unknown memory values in smdk2442_val.h
#endif

#define vMRSRB6	0x00000030
#define vMRSRB7	0x00000030

#endif /* _S3C2440_SMDK_H */

