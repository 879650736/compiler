	.text
	.globl main
main:
	li t0, 3
	xor t0, t0, 0
	seqz t0, t0
	li t1, 0
	li t2, 7
	sub t3, t1, t2
	li t4, 4
	slt t5, t4, t0
	xori t5, t5, 1
	li t6, 4
	xor t0, t6, t0
	snez t0, t0
	or t1, t0, t4
	xor t1, t0, t4
	snez t1, t1
	li t2, 2
	mul t3, t2, t1
	li t4, 1
	add t5, t4, t3
	li t6, 5
	slt t0, t6, t4
	li t1, 6
	xor t2, t1, t6
	snez t2, t2
	li t3, 0
	xor t4, t3, t1
	snez t4, t4
	li t5, 0
	xor t6, t5, t1
	snez t6, t6
	mv a0, t6
	ret
