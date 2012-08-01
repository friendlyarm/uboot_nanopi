#ifndef _S3C2440_SMDK_H
#define _S3C2440_SMDK_H

#include <config.h>
#include <version.h>

#include <s3c2440.h>

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

#define UPLL_VAL       ((MDIV_48 << 12) | (PDIV_48 << 4) | (SDIV_48))
#define MPLL_VAL       ((MDIV_406 << 12) | (PDIV_406 << 4) | (SDIV_406))
#define CLKDIVN_VAL		CLKDIVN_148
#elif (CONFIG_SYS_CLK_FREQ == 16934400)
  /* In 533, you must use CLKDIVN_148 instead of CLKDIVN_136 */
  //#define CONFIG_CLK_533_133_66
  #define CONFIG_CLK_406_135_67

  #ifdef CONFIG_SMDK2440
    #ifdef CONFIG_CLK_533_133_66
      #define MPLL_VAL	0x00076021
    #elif defined (CONFIG_CLK_406_135_67)
      #define MPLL_VAL	0x000b8022
    #endif
  #elif defined (CONFIG_SMDK2442)
    #define MPLL_VAL	0x00061012	/* 296, 98, 49 */
  #else
    #error system is not defined in smdk2440_val.h.
  #endif
  #define UPLL_VAL	0x0003c041
  #define CLKDIVN_VAL	0xf
#else
#error unknown system clock in smdk2440_val.h
#endif

#if defined (CONFIG_SMDK2440)
  #define vBANKCON6	0x00018005
  #define vBANKCON7	0x00018005
  #define vREFRESH	0x008e0459	/* period=15.6us, HCLK=60Mhz, (2048+1-15.6*60) */
  #define vBANKSIZE	0x00000032
  #define vMRSRB6	0x00000030
  #define vMRSRB7	0x00000030
#elif defined (CONFIG_SMDK2442)
  #if defined (CONFIG_SMDK2442_MSP1)
    #define vBANKCON6	0x00018004
    #define vBANKSIZE	0x00000030
  #elif defined (CONFIG_SMDK2442_MSP3)
    #define vBANKCON6	0x00018005
    #define vBANKSIZE	0x00000031
  #endif
  #define vBANKCON7	0x00018005
  #define vREFRESH	0x008e04f5
#endif

#define vBWSCON		0x2211d120
#define vBANKCON0	0x00000700
#define vBANKCON1	0x00000700
#define vBANKCON2	0x00000700
#define vBANKCON3	0x00001f4c
#define vBANKCON4	0x00000700
#define vBANKCON5	0x00000700

#define vMRSRB6	0x00000030
#define vMRSRB7	0x00000030

#endif /* _S3C2440_SMDK_H */

