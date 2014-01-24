	.text
	.p2align 4,,15
	.att_syntax prefix
	xchgl	%eax, %ebx
	fadd	%st, %st(1)	## dc c1 - intel: fadd st(1), st(0)
	fadd	%st(1), %st	## d8 c1 - intel: fadd st(0), st(1)
	faddp	%st, %st(1)
	.intel_syntax noprefix
	xchg	ebx, eax
	fadd	st, st(1)	## d8 c1 - intel: fadd st(0), st(1)
	fadd	st(1), st	## dc c1 - intel: fadd st(1), st(0)
	faddp	st(1), st

################################################################
	.att_syntax prefix
	xchgl	%eax, %ebx
	fsub	%st, %st(1)	## dc e1 - intel: fsubr st(1), st(0)
				## i.e. st(1) := st(0) - st(1)
	fsub	%st(1), %st	## d8 e1 - intel: fsub st(0), st(1)
				## i.e. st(0) := st(0) - st(1)
	fsubr	%st, %st(1)	## dc e9 - intel: fsub st(1), st(0)
				## i.e. st(1) := st(1) - st(0)
	fsubr	%st(1), %st	## d8 e9 - intel: fsubr st(0), st(1)
				## i.e. st(0) := st(1) - st(0)
	fsubp	%st, %st(2)	## de e2 - intel: fsubrp st(2), st(0)
				## i.e. st(2) := st(0) - st(2); pop st(0)
	fsubrp	%st, %st(2)	## de ea - intel: fsubp st(2), st(0)
				## i.e. st(2) := st(2) - st(0); pop st(0)
	.intel_syntax noprefix
	xchg	ebx, eax
	fsub	st, st(1)	## d8 e1 - att: fsub %st(1), %st
				## i.e. st(0) := st(0) - st(1)
	fsub	st(1), st	## dc e9 - att: fsubr %st, %st(1)
				## i.e. st(1) := st(1) - st(0)
	fsubr	st, st(1)	## d8 e9 - att: fsubr %st(1), %st
				## i.e. st(0) := st(1) - st(0)
	fsubr	st(1), st	## dc e1 - att: fsub %st, %st(1)
				## i.e. st(1) := st(0) - st(1)
	fsubp	st(2), st	## de ea - att: fsubrp %st, %st(2)
				## i.e. st(2) := st(2) - st(0); pop st(0)
	fsubrp	st(2), st	## de e2 - att: fsubp  %st, %st(2)
				## i.e. st(2) := st(0) - st(2); pop st(0)

################################################################
	.att_syntax prefix
	xchgl	%eax, %ebx
	fdiv	%st, %st(1)	## dc f9 - intel: fdivr st(1), st(0)
				## i.e. st(1) := st(0) - st(1)
	fdiv	%st(1), %st	## d8 f9 - intel: fdiv st(0), st(1)
				## i.e. st(0) := st(0) - st(1)
	fdivr	%st, %st(1)	## dc e9 - intel: fdiv st(1), st(0)
				## i.e. st(1) := st(1) - st(0)
	fdivr	%st(1), %st	## d8 e9 - intel: fdivr st(0), st(1)
				## i.e. st(0) := st(1) - st(0)
	.intel_syntax noprefix
	xchg	ebx, eax
	fdiv	st, st(1)	## d8 f9 - intel: fdiv st(0), st(1)
				## i.e. st(0) := st(0) - st(1)
	fdiv	st(1), st	## dc e9 - intel: fdiv st(1), st(0)
				## i.e. st(1) := st(1) - st(0)
	fdivr	st, st(1)	## d8 e9 - intel: fdivr st(0), st(1)
				## i.e. st(0) := st(1) - st(0)
	fdivr	st(1), st	## dc f9 - intel: fdivr st(1), st(0)
				## i.e. st(1) := st(0) - st(1)
