	.verstamp 3 11
	.set noreorder
	.set volatile
	.set noat
	.arch ev5
	.file	1 "alpha-sincos.s"
gcc2_compiled.:
__gnu_compiled_c:
.text
	.align 3
	.globl alpha_sincos
	.ent alpha_sincos
alpha_sincos:
	ldgp $29,0($27)
alpha_sincos..ng:
	lda $30,-48($30)
	.frame $15,48,$26,0
	stq $26,0($30)
	stq $15,8($30)
	.mask 0x4008000,-48
	bis $30,$30,$15
	.prologue 1
	stt $f16,16($15)
	stq $17,24($15)
	stq $18,32($15)

	ldt $f16,16($15)
	jsr $26,sincos
	ldgp $29,0($26)
	ldq $1,24($15)
	stt $f0,0($1)

	ldq $1,32($15)
	stt $f1,0($1)

	bis $15,$15,$30
	ldq $26,0($30)
	ldq $15,8($30)
	addq $30,48,$30
	ret $31,($26),1
	.end alpha_sincos
