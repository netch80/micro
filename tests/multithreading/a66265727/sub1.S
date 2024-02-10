	.arch armv8-a
	.text
	.global _Z13exchange_lrscPSt6atomicIbEb
_Z13exchange_lrscPSt6atomicIbEb:
	// x0 - address
	// x1 - new value to set
1:	ldaxrb	w2, [x0]
//	stlxrb	w3, w1, [x0]
	stxrb	w3, w1, [x0]
	cbnz	w3, 1b
//-	ldarb	w3, [x0]
//-	dmb	ld
//-	dmb	st
//-	dsb	st

	mov	w0, w2
	ret
