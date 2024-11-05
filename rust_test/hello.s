	.text
	.globl main
main:
	li t0, 3
	li t1, 0
	xor t2, t0, t1
	seqz t0, t2

	li t1, 0
	li t2, 7
	sub t1, t1, t2

	li t2, 4
	li t3, 3
	li t4, 0
	xor t5, t3, t4
	seqz t3, t5

	sgt t4, t3, t2
	xori t2, t4, 1

	li t3, 4
	li t4, 3
	li t5, 0
	xor t6, t4, t5
	seqz t4, t6

	xor t5, t4, t3
	snez t3, t5

	li t4, 4
	li t5, 3
	li t6, 0
	xor t0, t5, t6
	seqz t5, t0

	sgt t6, t5, t4
	xori t4, t6, 1

	li t5, 4
	li t6, 3
	li t0, 0
	xor t1, t6, t0
	seqz t6, t1

	xor t0, t6, t5
	snez t5, t0

	or t4, t4, t5

	li t5, 2
	li t6, 3
	li t0, 0
	xor t1, t6, t0
	seqz t6, t1

	mul t5, t5, t6

	li t6, 1
	li t0, 2
	li t1, 3
	li t2, 0
	xor t3, t1, t2
	seqz t1, t3

	mul t0, t0, t1

	add t6, t6, t0

	li t0, 5
	li t1, 1
	li t2, 2
	li t3, 3
	li t4, 0
	xor t5, t3, t4
	seqz t3, t5

	mul t2, t2, t3

	add t1, t1, t2

	slt t0, t1, t0

	li t1, 6
	li t2, 5
	li t3, 1
	li t4, 2
	li t5, 3
	li t6, 0
	xor t0, t5, t6
	seqz t5, t0

	mul t4, t4, t5

	add t3, t3, t4

	slt t2, t3, t2

	xor t3, t2, t1
	snez t1, t3

	li t2, 0
	li t3, 6
	li t4, 5
	li t5, 1
	li t6, 2
	li t0, 3
	li t1, 0
	xor t2, t0, t1
	seqz t0, t2

	mul t6, t6, t0

	add t5, t5, t6

	slt t4, t5, t4

	xor t5, t4, t3
	snez t3, t5

	xor t4, t3, t2
	snez t2, t4

	li t3, 0
	li t4, 0
	li t5, 7
	sub t4, t4, t5

	xor t5, t4, t3
	snez t3, t5


	mv a0, t3
	ret
