#include <config.h>
#include <s3c2460.h>

#if defined (CONFIG_USE_MDDR_SDRAM)
/*
 * ddr_ram_asm_init: Initialize mDDR SDRAM.
 */
	.globl ddr_ram_asm_init
ddr_ram_asm_init:
	mov	r1, pc
	ldr	r2, =0x00000fff
	orr	r1, r1, r2
	and	r0, r0, r1

	/* read setting value from structure */
	ldmia r0!, {r5-r9}

	/* Step 1: set mem_sel, CL and DW */
	str	r6, [r5, #SDRAM_BANKCFG_OFFSET]

	/* Step 2: issue PALL */
	orr	r1, r7, #INIT_PALL
	str	r1, [r5, #SDRAM_BANKCON_OFFSET]

	/* Step 3: wait more than 2 cycles */
	nop
	nop
	nop
	nop

	/* Step 4: DLL_En -> 0, issue EMRS */
	orr	r1, r7, #INIT_EMRS
	str	r1, [r5, #SDRAM_BANKCON_OFFSET]

	/* Step 5: wait more than 2 cycles */
	nop
	nop
	nop
	nop

	/* Step 6: DLL_Re -> 1, issue MRS */
	orr	r1, r6, #(0x1<<23)
	str	r1, [r5, #SDRAM_BANKCFG_OFFSET]

	orr	r1, r7, #INIT_MRS
	str	r1, [r5, #SDRAM_BANKCON_OFFSET]

	/* Step 7: wait more than 2 cycles */
	nop
	nop
	nop
	nop

	/* Step 8: issue PALL */
	orr	r1, r7, #INIT_PALL
	str	r1, [r5, #SDRAM_BANKCON_OFFSET]

	/* Step 9: write 0xf into the refresh timer */
	mov	r1, #0xf
	str	r1, [r5, #SDRAM_REFRESH_OFFSET]


	/* Step 10: wait 200-clock cycles for DLL lock time (14 refresh cycles) */
	mov	r1, #200
1:	subs	r1, r1, #1
	bne	1b

	/* Step 11: DLL_Re -> 0, issue MRS */
	str	r6, [r5, #SDRAM_BANKCFG_OFFSET]

	orr	r1, r7, #INIT_MRS
	str	r1, [r5, #SDRAM_BANKCON_OFFSET]

	/* Step 12: nornal operation value into the refresh timer */
	str	r8, [r5, #SDRAM_REFRESH_OFFSET]

	/* Step 13: Normal Mode */
	str	r7, [r5, #SDRAM_BANKCON_OFFSET]
	str	r6, [r5, #SDRAM_BANKCFG_OFFSET]

	mov	pc, lr
#endif

#if defined (CONFIG_USE_MSDR_SDRAM)
@
@ sdr_ram_asm_init: Initialize memory controller
@
	.globl sdr_ram_asm_init
sdr_ram_asm_init:
	mov	r1, pc
	ldr	r2, =0x00000fff
	orr	r1, r1, r2
	and	r0, r0, r1

	/* read setting value from structure */
	ldmia r0!, {r5-r9}

	/* Step 1: issue PALL */
	orr	r1, r7, #INIT_PALL
	str	r1, [r5, #SDRAM_BANKCON_OFFSET]

	/* Step 2: write 0xf into the refresh timer */
	mov	r1, #0xf
	str	r1, [r5, #SDRAM_REFRESH_OFFSET]

	/* Step 3: wait 120-clock cycles(8 refresh cycles) */
	mov	r1, #120
1:	subs	r1, r1, #1
	bne	1b

	/* Step 4: nornal operation value into the refresh timer */
	str	r8, [r5, #SDRAM_REFRESH_OFFSET]

	/* Step 5: set mem_sel, CL and DW */
	str	r6, [r5, #SDRAM_BANKCFG_OFFSET]

	/* Step 6: issue MRS */
	orr	r1, r7, #INIT_MRS
	str	r1, [r5, #SDRAM_BANKCON_OFFSET]

	/* Step 7: issue EMRS */
	orr	r1, r7, #INIT_EMRS
	str	r1, [r5, #SDRAM_BANKCON_OFFSET]

	/* Step 8: Normal Mode */
	str	r7, [r5, #SDRAM_BANKCON_OFFSET]
	str	r6, [r5, #SDRAM_BANKCFG_OFFSET]

	mov	pc, lr
#endif

/* Below code is for ARM926EJS and ARM1026EJS */
	.globl cleanDCache
cleanDCache:
	mrc	p15, 0, pc, c7, c10, 3	/* test/clean D-Cache */
	bne	cleanDCache
	mov	pc, lr

	.globl cleanFlushDCache
cleanFlushDCache:
	mrc	p15, 0, pc, c7, c14, 3	/* test/cleanflush D-Cache */
	bne	cleanFlushDCache
	mov	pc, lr

	.globl cleanFlushCache
cleanFlushCache:
	mrc	p15, 0, pc, c7, c14, 3	/* test/cleanflush D-Cache */
	bne	cleanFlushCache
	mcr	p15, 0, r0, c7, c5, 0	/* flush I-Cache */
	mov	pc, lr

	.ltorg

