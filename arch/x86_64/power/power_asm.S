.intel_syntax noprefix
.text

.globl power_triple_fault
.type power_triple_fault, @function
power_triple_fault:
	cli
	lidt [rip + power_null_idtr]
	int3
1:
	hlt
	jmp 1b

.section .rodata
.balign 16
power_null_idtr:
	.word 0
	.quad 0

.section .note.GNU-stack,"",@progbits
