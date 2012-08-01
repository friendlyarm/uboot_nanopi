#include <config.h>
#include <s3c2412.h>

#define BANKCFG_OFFSET	0x00
#define BANKCON1_OFFSET	0x04
#define BANKCON2_OFFSET	0x08
#define BANKCON3_OFFSET	0x0c
#define REFRESH_OFFSET	0x10
#define TIMEOUT_OFFSET	0x14


@
@ sdr_ram_asm_init: Initialize memory controller
@
	.globl sdr_ctrl_asm_init
sdr_ctrl_asm_init:
	mov	r1, pc
	ldr	r2, =0x00000fff
	orr	r1, r1, r2
	and	r0, r0, r1

	/* read setting value from structure */
	ldr	r4, =ELFIN_MEMCTL_BASE
	ldmia	r0!, {r5-r9}
	stmia	r4!, {r5-r9}

#ifdef CONFIG_PM
        /* compare INFORM2 register to confirm the dram type*/
        ldr     r1, =INFORM2_REG
        ldr     r1, [r1]
        ldr     r2, =0xDA
        cmp     r2, r1
        
        ldreq   r4, =ELFIN_MEMCTL_BASE
        beq     1002f
#endif  

1000:	ldr	r4, =ELFIN_MEMCTL_BASE

	/* Step 1: issue PALL */
	ldr	r1, [r4, #BANKCON1_OFFSET]
	bic	r1, r1, #0x3
	orr	r1, r1, #0x1
	str	r1, [r4, #BANKCON1_OFFSET]

	/* Step 2: write 0xff into the refresh timer */
	mov	r1, #0xff
	str	r1, [r4, #REFRESH_OFFSET]

	/* Step 3: wait more than 120 clk */
	mov	r1, #0x100
1:	subs	r1, r1, #1
	bne	1b

	/* Step 4: issue MRS */
	ldr	r1, [r4, #BANKCON1_OFFSET]
	bic	r1, r1, #0x3
	orr	r2, r1, #0x2
	str	r2, [r4, #BANKCON1_OFFSET]

	/* Step 5: nornal operation value into the refresh timer */
	str	r9, [r4, #REFRESH_OFFSET]

	/* Step 6: issue EMRS */
	orr	r2, r1, #0x3
	str	r2, [r4, #BANKCON1_OFFSET]

	/* Step 7: Normal Mode */
	orr	r2, r1, #0x0
	str	r2, [r4, #BANKCON1_OFFSET]

#ifdef CONFIG_SDRAM_AUTO_DETECT
	ldr 	r6, =PHYS_SDRAM_1
	ldr	r7, =0x24564236
	swp	r8, r7, [r6]
	swp	r5, r8, [r6]
	cmp	r7, r5
	beq	1001f

1002:
	ldr	r5, =CFG_BANK_CFG_VAL_ALT
	ldr	r6, =CFG_BANK_CON2_VAL_ALT
	str	r5, [r4]
	str	r6, [r4, #BANKCON2_OFFSET]
	
	b	1000b

1001:
#endif

	mov	pc, lr

	.ltorg

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

