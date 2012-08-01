#include <config.h>
#include <s3c2442.h>

	/* set memory control registers */
	.globl mem_con_init
mem_con_init:
	/* you must make sure that this sequence is unusual in dram
	 * initialization. if you want to find normal sequence
	 * check 2443/2413 as reference. by scsuh.
	 */
	mov     r1, #ELFIN_MEM_CON_BASE
	add     r3, r1, #52
1:	ldr     r4, [r0], #4
	str     r4, [r1], #4
	cmp     r1, r3
	bne     1b
	mov     pc, lr

/* Below code is for ARM920T */
	.globl cleanDCache
cleanDCache:
	mcr	p15, 0, r0, c7, c10, 2
	add	r0, r0, #(1<<5)
	tst	r0, #(1<<7)
	beq	cleanDCache
	bic	r0, r0, #(1<<7)
	adds	r0, r0, #(1<<26)

	bcc	cleanDCache
	mov	pc, lr

	.globl cleanFlushDCache
cleanFlushDCache:
	mcr	p15, 0, r0, c7, c14, 2
	add	r0, r0, #(1<<5)
	tst	r0, #(1<<7)
	beq	cleanFlushDCache
	bic	r0, r0, #(1<<7)
	adds	r0, r0, #(1<<26)

	bcc	cleanFlushDCache
	mov	pc, lr

	.globl cleanFlushCache
cleanFlushCache:
	mrc	p15, 0, pc, c7, c14, 3	/* test/cleanflush D-Cache */
	bne	cleanFlushCache
	mcr	p15, 0, r0, c7, c5, 0	/* flush I-Cache */
	mov	pc, lr

	.ltorg

