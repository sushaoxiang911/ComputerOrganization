	lw	0	1	n
	lw	0	2	r
	lw	0	6	fadd
	jalr	6	7	
	halt
func	sw	5	7	stack
	lw	0	6	pos1		change the temp save to be 1
	add	5	6	5		move the pointer downward
	sw	5	1	stack		save input(n)
	add	5	6	5		move the pointer downward
	sw	5	2	stack		save input(r)
	add	5	6	5
	beq	0	2	ret0		if n=0, goto ret0,else goto next check
	beq	1	2	ret0		else if r=0, goto ret0,else got next step
	lw	0	6	neg1
	add	1	6	1
	lw	0	6	fadd
	jalr	6	7
	lw	0	6	neg1	
	add	2	6	2
	lw	0	6	fadd
	jalr	6	7
	beq	0	0	ret
ret0	lw	0	6	pos1
	add	3	6	3
ret	lw	0	6	neg1
	add	5	6	5
	lw	5	2	stack
	add	5	6	5
	lw	5	1	stack
	add	5	6	5
	lw	5	7	stack
	jalr	7	6	
n	.fill	6
r	.fill	3
fadd	.fill	func
pos1	.fill	1
neg1	.fill	-1
stack	.fill	0
