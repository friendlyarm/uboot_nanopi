#include <config.h>
#include <regs.h>

#include "../../../board/samsung/smdk24a0/smdk24a0_val.h"

#define	REFRESHRATES_VAL	360


	.globl mem_con_init
mem_con_init:
	ldr	r0, =ELFIN_SROM_BASE
	adrl	r1, SROM_val
	ldmia	r1!, {r2-r5}
	stmia	r0!, {r2-r5}

	/*
	 * now, we start sdram init.
	 */
	ldr	r0, =ELFIN_MEMCTL_BASE

	/* Step 1: issue precharge all command */
	mov	r1, #0x1
	str	r1, [r0, #SDRAM_BANKCON_OFFSET]

	/* Step 2: make refresh cycle 15clk */
#ifdef OREIAS
	mov	r1, #0x20
#else
	mov	r1, #0xf
#endif
	str	r1, [r0, #SDRAM_REFRESH_OFFSET]

	@ Step 3: wait 120 clk
	mov	r1, #0x100
1:	subs	r1, r1, #1
	bne	1b

	@ Step 4: set normal operation refresh cycle
	ldr	r1,  =REFRESHRATES_VAL
	str	r1, [r0, #SDRAM_REFRESH_OFFSET]

	@ Step 5: set cfg/ctrl/timeout registers
	ldr	r1, SDRAM_val
	str	r1, [r0, #0x0]
	mov	r1, #0x0
	str	r1, [r0, #SDRAM_BANKCON_OFFSET]	@ memcon. WB off, Normal
	str	r1, [r0, #SDRAM_TIMEOUT_OFFSET]	@ disable
	@ Step 6: MRS command
	mov	r1, #0x2
	str	r1, [r0, #SDRAM_BANKCON_OFFSET]
#ifdef OREIAS
	@ Step 7: issue EMRS command (only mobile)
	mov	r1, #0x3
	str	r1, [r0, #SDRAM_BANKCON_OFFSET]
#endif
	@ Step 8: Normal operation
	mov	r1, #0x0
	str	r1, [r0, #SDRAM_BANKCON_OFFSET]

	mov	pc, lr

sdram_init_val:
	.ltorg
SROM_val:		.long	SROM_BW_VAL
			.long	SROM_BANK0_VAL
			.long	SROM_BANK1_VAL
			.long	SROM_BANK2_VAL
SDRAM_val:		.long	SDRAM_BANK0_VAL

