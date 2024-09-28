	.text
	.globl main
main:
	li t0, 1
	li t1, 2
	sgt t1, t0, t1
	sgt t1, t1
	mv a0, t1
	ret
