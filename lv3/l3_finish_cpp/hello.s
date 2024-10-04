	.text
	.globl main
main:
	li t0, 9
	li t1, 3
	slt t2, t0, t1
	xori t0, t2, 1

	mv a0, t0
	ret
