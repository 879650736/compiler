	.text
	.globl main
main:
	li t0, 3
	li t1, 0
	xor t2, t1, t0
	snez t0, t2

	li t1, 0
	li t2, 7
	xor t3, t2, t1
	snez t1, t3

	li t2, 4
	xor t3, t2, t0
	snez t2, t3

	li t3, 2
	xor t4, t0, t3
	snez t3, t4

	xor t4, t3, t2
	snez t4, t4

	li t5, 3
	li t6, 0
	xor t0, t6, t5
	snez t5, t0

	li t6, 0
	li t0, 7
	xor t1, t0, t6
	snez t6, t1

	li t0, 4
	li t1, 3
	li t2, 0
	xor t3, t2, t1
	snez t1, t3

	xor t2, t1, t0
	snez t0, t2

	li t1, 2
	li t2, 3
	li t3, 0
	xor t4, t3, t2
	snez t2, t4

	xor t3, t1, t2
	snez t1, t3

	li t2, 4
	li t3, 3
	li t4, 0
	xor t5, t4, t3
	snez t3, t5

	xor t4, t3, t2
	snez t2, t4

	xor t3, t3, t1
	snez t2, t3

	xor t4, t1, t0
	snez t4, t4

	mv a0, t3
	ret
