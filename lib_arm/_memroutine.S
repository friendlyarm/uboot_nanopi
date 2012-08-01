/* add to check performance in 64xx
 * by scsuh.
 */
/* ip		.req	r12	*/
/* sp		.req	r13	*/
/* lr		.req	r14	*/
/* pc		.req	r15	*/

	.text
	.globl	 write_multiple_pld
write_multiple_pld      :
	stmfd	sp!, {r5 - r8, lr} /* backup lr register */
	mov	r3, r1
	mov	ip, r1
	mov	lr, r1
	mov	r5, r1
	mov	r6, r1
	mov	r7, r1
	mov	r8, r1
	pld	[r0]
	pld	[r0, #28]
	pld	[r0, #60]
	pld	[r0, #92]
1:	subs	r2, r2, #32	/* size is bigger than 32? */
	pld	[r0, #124]
	stmia	r0!, {r1, r3, r5, r6, r7, r8, ip, lr}
	bgt	1b
	ldmfd	sp!, {r5 - r8, pc}

	.globl	 write_multiple
write_multiple      :
	stmfd	sp!, {r5 - r8, lr} /* backup lr register */
	mov	r3, r1
	mov	ip, r1
	mov	lr, r1
	mov	r5, r1
	mov	r6, r1
	mov	r7, r1
	mov	r8, r1
1:	subs	r2, r2, #32	/* size is bigger than 32? */
	stmia	r0!, {r1, r3, r5, r6, r7, r8, ip, lr}
	bgt	1b
	ldmfd	sp!, {r5 - r8, pc}


	.globl	 read_multiple_pld
read_multiple_pld      :
	stmfd	sp!, {r5 - r8, lr} /* backup lr register */
	pld	[r0]
	pld	[r0, #28]
	pld	[r0, #60]
	pld	[r0, #92]
1:	subs	r2, r2, #32	/* size is bigger than 32? */
	pld	[r0, #124]
	ldm	r0!, {r1, r3, r5, r6, r7, r8, ip, lr}
	bgt	1b
	ldmfd	sp!, {r5 - r8, pc}

	.globl	 read_multiple
read_multiple:
	stmfd	sp!, {r5 - r8, lr} /* backup lr register */
1:	subs	r2, r2, #32	/* size is bigger than 32? */
	ldm	r0!, {r1, r3, r5, r6, r7, r8, ip, lr}
	bgt	1b
	ldmfd	sp!, {r5 - r8, pc}

	.globl	 write_str_pld
write_str_pld:
	stmfd	sp!, {r5 - r8, lr} /* backup lr register */
	pld	[r0]
	pld	[r0, #28]
	pld	[r0, #60]
	pld	[r0, #92]
1:	subs	r2, r2, #32	/* size is bigger than 32? */
	pld	[r0, #124]
	str     r1, [r0], #4
	str     r1, [r0], #4
	str     r1, [r0], #4
	str     r1, [r0], #4
	str     r1, [r0], #4
	str     r1, [r0], #4
	str     r1, [r0], #4
	str     r1, [r0], #4
	bgt	1b
	ldmfd	sp!, {r5 - r8, pc}

	.globl	 write_str
write_str:
	stmfd	sp!, {r5 - r8, lr} /* backup lr register */
1:	subs	r2, r2, #32	/* size is bigger than 32? */
	str     r1, [r0], #4
	str     r1, [r0], #4
	str     r1, [r0], #4
	str     r1, [r0], #4
	str     r1, [r0], #4
	str     r1, [r0], #4
	str     r1, [r0], #4
	str     r1, [r0], #4
	bgt	1b
	ldmfd	sp!, {r5 - r8, pc}

	.globl	 read_ldr_pld
read_ldr_pld:
	stmfd	sp!, {r5 - r8, lr} /* backup lr register */
	pld	[r0]
	pld	[r0, #28]
	pld	[r0, #60]
	pld	[r0, #92]
1:	subs	r2, r2, #32	/* size is bigger than 32? */
	pld	[r0, #124]
	ldr     r1, [r0], #4
	ldr     r1, [r0], #4
	ldr     r1, [r0], #4
	ldr     r1, [r0], #4
	ldr     r1, [r0], #4
	ldr     r1, [r0], #4
	ldr     r1, [r0], #4
	ldr     r1, [r0], #4
	bgt	1b
	ldmfd	sp!, {r5 - r8, pc}

	.globl	 read_ldr
read_ldr:
	stmfd	sp!, {r5 - r8, lr} /* backup lr register */
1:	subs	r2, r2, #32	/* size is bigger than 32? */
	ldr     r1, [r0], #4
	ldr     r1, [r0], #4
	ldr     r1, [r0], #4
	ldr     r1, [r0], #4
	ldr     r1, [r0], #4
	ldr     r1, [r0], #4
	ldr     r1, [r0], #4
	ldr     r1, [r0], #4
	bgt	1b
	ldmfd	sp!, {r5 - r8, pc}

