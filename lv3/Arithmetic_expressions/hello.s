	.text
	.globl main
main:
	li t0, 2
	li t1, 3
	mul t1, t0, t1
	li t2, 1
	add t2, t1, t2
	mv a0, t2
	ret
